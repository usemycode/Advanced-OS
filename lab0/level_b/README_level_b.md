# Producer-Consumer Problem Implementation in C

## Overview

This program is an implementation of the **Producer-Consumer Problem** using semaphores and shared memory in C. The goal is to simulate a scenario where producers generate items and store them in a shared buffer, while consumers take items from the buffer to consume them. Semaphores are used for synchronization to ensure proper coordination between the producers and consumers.

## Key Concepts

1. **Shared Memory**: Used to store the buffer and the count of items in the buffer.
2. **Semaphores**: Control access to the buffer, ensuring that producers and consumers don't access it simultaneously.
3. **Process-based Concurrency**: Child processes are created using `fork()` to handle item production and consumption.

## Key Variables

- `buffer`: A shared array that holds the items.
- `count`: A shared variable tracking the number of items currently in the buffer.
- `empty_slots`: A semaphore tracking the number of empty slots in the buffer.
- `filled_slots`: A semaphore tracking the number of filled slots in the buffer.
- `mutex`: A binary semaphore ensuring exclusive buffer access.

## Detailed Code Breakdown

### 1. Header Files and Definitions

```c
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
```

### 2. Shared Memory Allocation

```c
int *buffer;                // shared buffer
int *count;                 // shared count of items in buffer

// Allocate shared memory
buffer = mmap(NULL, BUFFER_SIZE * sizeof(int), PROT_READ | PROT_WRITE,
              MAP_SHARED | MAP_ANONYMOUS, -1, 0);
count = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
*count = 0;  // initialize count
```

### 3. Semaphore Initialization

```c
// Initialize semaphores
empty_slots = sem_open("/empty_slots", O_CREAT | O_EXCL, 0666, BUFFER_SIZE);
filled_slots = sem_open("/filled_slots", O_CREAT | O_EXCL, 0666, 0);
mutex = sem_open("/mutex", O_CREAT | O_EXCL, 0666, 1);
```

### 4. Producer Function

```c
void producer(int id) {
    int item, i = 0;
    while (i < 5) {  // Each producer produces 5 items
        item = rand() % 100;  // produce an item

        sem_wait(empty_slots);  // wait for an empty slot
        sem_wait(mutex);        // lock buffer for exclusive access

        // Add item to buffer
        buffer[(*count)++] = item;
        printf("Producer %d produced: %d\n", id, item);

        sem_post(mutex);        // unlock buffer
        sem_post(filled_slots); // signal that there is a new filled slot

        i++;
        sleep(1);
    }
}
```

### 5. Consumer Function

```c
void consumer(int id) {
    int item, i = 0;
    while (i < 5) {  // Each consumer consumes 5 items
        sem_wait(filled_slots); // wait for a filled slot
        sem_wait(mutex);        // lock buffer for exclusive access

        // Remove item from buffer
        item = buffer[--(*count)];
        printf("Consumer %d consumed: %d\n", id, item);

        sem_post(mutex);        // unlock buffer
        sem_post(empty_slots);  // signal that there is a new empty slot

        i++;
        sleep(2);
    }
}
```

### 6. Process Creation and Synchronization

```c
int main() {
    // Create producer processes
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            producer(i + 1);
            exit(0);
        }
    }

    // Create consumer processes
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            consumer(i + 1);
            exit(0);
        }
    }

    // Wait for all child processes to finish
    for (int i = 0; i < NUM_PRODUCERS + NUM_CONSUMERS; i++) {
        wait(NULL);
    }

    // Cleanup
    sem_unlink("/empty_slots");
    sem_unlink("/filled_slots");
    sem_unlink("/mutex");
    munmap(buffer, BUFFER_SIZE * sizeof(int));
    munmap(count, sizeof(int));

    return 0;
}
```

## Synchronization Mechanism

### Semaphore Usage

- `empty_slots`: Prevents producers from adding items when the buffer is full.
- `filled_slots`: Prevents consumers from removing items when the buffer is empty.
- `mutex`: Ensures exclusive access to the shared buffer.

## Workflow

1. Producers generate random items.
2. Producers wait for an empty slot in the buffer.
3. Producers lock the buffer and add an item.
4. Consumers wait for a filled slot in the buffer.
5. Consumers lock the buffer and remove an item.
6. The process repeats until each producer and consumer completes their tasks.

## Conclusion

This implementation demonstrates:
- Inter-process communication
- Shared memory management
- Semaphore-based synchronization
- Process creation and management

The code showcases how to solve the classic Producer-Consumer problem using low-level system programming techniques in C.

## Compilation and Execution

```bash
gcc -o level_b level_b.c -pthread
./level_b
```

