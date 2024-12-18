#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>

#define BUFFER_SIZE 4
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2

int *buffer;                // shared buffer
int *count;                 // shared count of items in buffer

// Semaphores for synchronization
sem_t *empty_slots;         // semaphore for empty slots in buffer
sem_t *filled_slots;        // semaphore for filled slots in buffer
sem_t *mutex;               // mutex for exclusive access to buffer

// Producer function
void producer(int id) {
    int item, i = 0;
    while (i < 5) {  // Each producer produces 5 items
        item = rand() % 100;  // produce an item

        printf("Producer %d: Checking for empty slot...\n", id);
        sem_wait(empty_slots);         // wait for an empty slot
        printf("Producer %d: Found empty slot.\n", id);

        printf("Producer %d: Trying to lock buffer...\n", id);
        sem_wait(mutex);               // lock buffer for exclusive access
        printf("Producer %d: Buffer locked.\n", id);

        // Add item to buffer
        buffer[(*count)++] = item;
        printf("Producer %d produced: %d | Buffer: ", id, item);
        for (int j = 0; j < *count; j++) printf("%d ", buffer[j]);
        printf("\n");

        sem_post(mutex);               // unlock buffer
        printf("Producer %d: Buffer unlocked.\n", id);

        sem_post(filled_slots);        // signal that there is a new filled slot
        printf("Producer %d: Signaled filled slot.\n", id);

        i++;
        sleep(1);
    }
}

// Consumer function
void consumer(int id) {
    int item, i = 0;
    while (i < 5) {  // Each consumer consumes 5 items
        printf("Consumer %d: Checking for filled slot...\n", id);
        sem_wait(filled_slots);        // wait for a filled slot
        printf("Consumer %d: Found filled slot.\n", id);

        printf("Consumer %d: Trying to lock buffer...\n", id);
        sem_wait(mutex);               // lock buffer for exclusive access
        printf("Consumer %d: Buffer locked.\n", id);

        // Remove item from buffer
        item = buffer[--(*count)];
        printf("Consumer %d consumed: %d | Buffer: ", id, item);
        for (int j = 0; j < *count; j++) printf("%d ", buffer[j]);
        printf("\n");

        sem_post(mutex);               // unlock buffer
        printf("Consumer %d: Buffer unlocked.\n", id);

        sem_post(empty_slots);         // signal that there is a new empty slot
        printf("Consumer %d: Signaled empty slot.\n", id);

        i++;
        sleep(2);
    }
}

int main() {
    pid_t pid;

    // Allocate shared memory for buffer and count
    buffer = mmap(NULL, BUFFER_SIZE * sizeof(int), PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    count = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *count = 0;  // initialize count

    // Initialize semaphores
    empty_slots = sem_open("/empty_slots", O_CREAT | O_EXCL, 0666, BUFFER_SIZE);
    filled_slots = sem_open("/filled_slots", O_CREAT | O_EXCL, 0666, 0);
    mutex = sem_open("/mutex", O_CREAT | O_EXCL, 0666, 1);

    // Create producer and consumer processes
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pid = fork();
        if (pid == 0) {
            producer(i + 1);
            exit(0);
        } else if (pid < 0) {
            perror("Fork failed for producer");
            exit(1);
        }
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pid = fork();
        if (pid == 0) {
            consumer(i + 1);
            exit(0);
        } else if (pid < 0) {
            perror("Fork failed for consumer");
            exit(1);
        }
    }

    // Wait for all child processes to finish
    for (int i = 0; i < NUM_PRODUCERS + NUM_CONSUMERS; i++) {
        wait(NULL);
    }

    // Clean up semaphores and shared memory
    sem_unlink("/empty_slots");
    sem_unlink("/filled_slots");
    sem_unlink("/mutex");
    munmap(buffer, BUFFER_SIZE * sizeof(int));
    munmap(count, sizeof(int));

    return 0;
}