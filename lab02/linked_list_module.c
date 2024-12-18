#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define PROC_FILENAME "linked_list_control"
#define BUFFER_SIZE 128

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("Kernel Module for Linked List with Parameterized Initial Nodes and Dynamic Add/Remove");

/* Structure to represent a node in the linked list */
struct my_node {
    int data;
    struct list_head list;
};

/* Linked list head */
static LIST_HEAD(my_list);

/* Module parameter for node count */
static int node_count = 5; // Default to 5 nodes
module_param(node_count, int, 0444);
MODULE_PARM_DESC(node_count, "Number of initial nodes to create");

/* Global node counter */
static int node_counter = 0;

/* Proc file buffer */
static char proc_buffer[BUFFER_SIZE];

/* Add a node to the linked list */
static void add_node(int data) {
    struct my_node *new_node;

    new_node = kmalloc(sizeof(struct my_node), GFP_KERNEL);
    if (!new_node) {
        printk(KERN_ERR "Linked List Module - Memory allocation failed\n");
        return;
    }

    new_node->data = data;
    list_add_tail(&new_node->list, &my_list);

    printk(KERN_INFO "Linked List Module - Node added with data: %d\n", data);
    node_counter++;
}

/* Remove the last node from the linked list */
static void remove_last_node(void) {
    struct my_node *last_node;

    if (list_empty(&my_list)) {
        printk(KERN_WARNING "Linked List Module - List is empty, no node to remove\n");
        return;
    }

    last_node = list_last_entry(&my_list, struct my_node, list);
    printk(KERN_INFO "Linked List Module - Removing node with data: %d\n", last_node->data);

    list_del(&last_node->list);
    kfree(last_node);
    node_counter--;
}

/* Proc file write handler */
static ssize_t proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos) {
    int input_data;
    char command[BUFFER_SIZE];

    if (count > BUFFER_SIZE - 1)
        return -EINVAL;

    /* Copy user input to kernel buffer */
    if (copy_from_user(proc_buffer, buffer, count))
        return -EFAULT;

    proc_buffer[count] = '\0'; // Null-terminate the input

    /* Parse the command */
    if (sscanf(proc_buffer, "add %d", &input_data) == 1) {
        add_node(input_data);
    } else if (strncmp(proc_buffer, "remove", 6) == 0) {
        remove_last_node();
    } else {
        printk(KERN_WARNING "Linked List Module - Invalid command\n");
    }

    return count;
}

/* Proc file read handler */
static ssize_t proc_read(struct file *file, char __user *buffer, size_t count, loff_t *pos) {
    struct my_node *current_node;
    char temp_buffer[BUFFER_SIZE];
    int len = 0;

    /* Return 0 if data has already been read */
    if (*pos > 0)
        return 0;

    /* Construct the data to be returned */
    len += snprintf(temp_buffer + len, BUFFER_SIZE - len, "Linked List Nodes:\n");

    list_for_each_entry(current_node, &my_list, list) {
        len += snprintf(temp_buffer + len, BUFFER_SIZE - len, "  Data: %d\n", current_node->data);
        if (len >= BUFFER_SIZE) {
            printk(KERN_WARNING "Linked List Module - Buffer overflow during read\n");
            return -EFAULT;
        }
    }

    if (len == strlen("Linked List Nodes:\n")) {
        len += snprintf(temp_buffer + len, BUFFER_SIZE - len, "  (No nodes in the list)\n");
    }

    /* Copy data to user space */
    if (copy_to_user(buffer, temp_buffer, len)) {
        printk(KERN_ERR "Linked List Module - Failed to copy data to user space\n");
        return -EFAULT;
    }

    /* Update the file position and return number of bytes read */
    *pos += len;
    return len;
}


/* Proc file operations */
static struct proc_ops proc_file_ops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

/* Module Initialization */
static int __init linked_list_module_init(void) {
    int i;
    struct proc_dir_entry *entry;

    printk(KERN_INFO "Linked List Module - Initializing\n");

    /* Create proc file */
    entry = proc_create(PROC_FILENAME, 0666, NULL, &proc_file_ops);
    if (!entry) {
        printk(KERN_ERR "Linked List Module - Failed to create proc file\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "Linked List Module - Proc file created: /proc/%s\n", PROC_FILENAME);

    /* Add initial nodes based on module parameter */
    for (i = 1; i <= node_count; i++) {
        add_node(i * 10); // Initialize with values like 10, 20, 30, ...
    }

    return 0;
}

/* Module Cleanup */
static void __exit linked_list_module_exit(void) {
    struct my_node *current_node, *tmp;

    printk(KERN_INFO "Linked List Module - Cleaning up\n");

    /* Traverse and delete all nodes */
    list_for_each_entry_safe(current_node, tmp, &my_list, list) {
        printk(KERN_INFO "Linked List Module - Deleting node with data: %d\n", current_node->data);
        list_del(&current_node->list);
        kfree(current_node);
    }

    /* Remove proc file */
    remove_proc_entry(PROC_FILENAME, NULL);
    printk(KERN_INFO "Linked List Module - Proc file removed\n");
}

module_init(linked_list_module_init);
module_exit(linked_list_module_exit);

