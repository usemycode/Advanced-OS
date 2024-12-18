# Producer-Consumer Problem Implementation

This project demonstrates the **Producer-Consumer Problem** using processes in C. It employs **shared memory** and **semaphores** for synchronization, ensuring a bounded buffer is accessed safely by both producer and consumer processes.

---

## Key Concepts

### **Shared Memory**
Shared memory is used to store the buffer and its associated metadata, allowing producer and consumer processes to communicate and share data efficiently.

### **Semaphores**
Semaphores ensure proper synchronization to avoid race conditions and maintain a consistent state of the buffer:
- **`empty_slots`**: Tracks available buffer slots for the producer.
- **`filled_slots`**: Tracks the number of items in the buffer for the consumer.
- **`mutex`**: Ensures mutual exclusion when accessing the buffer.

### **Processes**
This implementation avoids threads and uses separate **producer** and **consumer** processes created via `fork()`.

---

## Code Overview

### **1. Shared Resources**

#### **Buffer**
- An array (`buffer`) of size `BUFFER_SIZE` to hold the items produced and consumed.
- Items are added by the producer and removed by the consumer.

#### **Count**
- A shared integer (`count`) to track the number of items currently in the buffer.

#### **Semaphores**
- **`empty_slots`**: Initialized to `BUFFER_SIZE`. Decreases when the producer adds items and increases when the consumer removes items.
- **`filled_slots`**: Initialized to `0`. Increases when the producer adds items and decreases when the consumer removes items.
- **`mutex`**: A binary semaphore (initialized to `1`) to ensure exclusive access to the buffer.

---

### **2. Workflow**

#### **Producer Process**
The producer adds items to the buffer by:
1. **Waiting for an empty slot**: Uses `sem_wait(empty_slots)`.
2. **Locking the buffer**: Ensures exclusive access using `sem_wait(mutex)`.
3. **Adding an item**: Places the item in the buffer and increments `count`.
4. **Unlocking the buffer**: Releases the lock using `sem_post(mutex)`.
5. **Signaling a filled slot**: Increments `filled_slots` using `sem_post(filled_slots)`.

#### **Consumer Process**
The consumer removes items from the buffer by:
1. **Waiting for a filled slot**: Uses `sem_wait(filled_slots)`.
2. **Locking the buffer**: Ensures exclusive access using `sem_wait(mutex)`.
3. **Removing an item**: Retrieves the item from the buffer and decrements `count`.
4. **Unlocking the buffer**: Releases the lock using `sem_post(mutex)`.
5. **Signaling an empty slot**: Increments `empty_slots` using `sem_post(empty_slots)`.

#### **Synchronization**
1. The producer **blocks** when the buffer is full (`empty_slots = 0`).
2. The consumer **blocks** when the buffer is empty (`filled_slots = 0`).
3. The `mutex` semaphore ensures that only one process (producer or consumer) modifies the buffer at a time.

---

### **3. Initialization**

- **Shared Memory**:
  - Allocated using `mmap` for the buffer and the `count` variable.
- **Semaphores**:
  - Created using `sem_open`.
  - Names (e.g., `/empty_slots`) are provided for proper cleanup.
- **Processes**:
  - `fork()` creates two processes:
    - **Child process**: Executes the `producer()` function.
    - **Parent process**: Executes the `consumer()` function.

---

### **4. Cleanup**

- The parent process waits for the producer to complete using `wait(NULL)`.
- Semaphores are unlinked using `sem_unlink`.
- Shared memory is released using `munmap`.

---

## Example Output

```plaintext
Producer: Produced 42 | Buffer size: 1
Consumer: Consumed 42 | Buffer size: 0
Producer: Produced 35 | Buffer size: 1
Producer: Produced 84 | Buffer size: 2
Consumer: Consumed 35 | Buffer size: 1
Producer: Produced 63 | Buffer size: 2
Consumer: Consumed 84 | Buffer size: 1
Consumer: Consumed 63 | Buffer size: 0
Producer: Produced 91 | Buffer size: 1
Consumer: Consumed 91 | Buffer size: 0
```

---

## Observations

1. **Synchronization**:
   - The producer and consumer alternate operations based on buffer availability.
   - Proper use of semaphores ensures no race conditions occur.

2. **FIFO Behavior**:
   - The buffer operates as a FIFO queue, ensuring items are consumed in the order they are produced.

3. **Scalability**:
   - This design can be extended to handle multiple producers and consumers (see Level B implementation).

---

## Key Features

1. **Process-Based Implementation**:
   - Uses processes (`fork`) rather than threads.
2. **Bounded Buffer**:
   - Buffer size is respected with semaphore-based synchronization.
3. **Robust Cleanup**:
   - Properly unlinks semaphores and releases shared memory to prevent resource leaks.

---

## Conclusion

This implementation demonstrates a robust solution to the **Producer-Consumer Problem** using processes, shared memory, and semaphores. It highlights key operating system concepts such as inter-process communication, synchronization, and memory management.

## Compilation and Execution

To compile and run the program, use the following commands:

```bash
gcc level_a.c -o level_a -pthread
./level_a