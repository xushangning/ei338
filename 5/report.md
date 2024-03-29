# EI338 Computer System Engineering: Project Report 5

517030910384 徐尚宁

## Introduction

In this project report, we implement our own thread pool in Java and present our solution to the producer-consumer problem with POSIX synchronization tools.

## Environment

- Arch Linux
- OpenJDK 13.0.1

## The Thread Pool

Our implementation of the thread pool makes use of the implicit monitor associated with each object in Java. Simply put, Java maintains mutual exclusion for access to a object through its methods marked with the keyword `synchronized`.

```java
class Callme {
  synchronized void call(String msg) {
    ...
  }
}
```

That is, when one thread is executing in a `synchronized` method of an object, no other thread can execute in all `synchronized` methods of that object. An implicit monitor provides this guarantee.

### The `ThreadPool` Class

All threads are kept in the array `pool`. Thread i is idle if `pool[i]` is `null`. Because access to the `add` method is synchronized, this function is implemented in a lock-free manner:

```java
public synchronized void add(Runnable task) {
  if (threadsCount < pool.length)
    for (int i = 0; i < pool.length; ++i)
      if (pool[i] == null) {
        if (!queue.isEmpty()) {
          queue.addLast(task);
          task = queue.removeFirst();
        }
        pool[i] = new MyThread(task, this, i);
        pool[i].start();
        ++threadsCount;

        break;
      }
  else
    queue.addLast(task);
}
```

We design a `dispatch` method that accepts an index into the `pool` array. After thread i's `Runnable` has been executed, it will call `dispatch` with its thread pool index i so that the thread pool dispatch a new job if there are still tasks in the queue, or mark that thread as completed/idle.

```java
public synchronized void dispatch(int threadPoolIndex) {
  if (!queue.isEmpty()) {
    Runnable t = queue.removeFirst();
    pool[threadPoolIndex] = new MyThread(t, this, threadPoolIndex);
    pool[threadPoolIndex].start();
  }
  else {
    // mark completed threads as null
    pool[threadPoolIndex] = null;
    --threadsCount;
  }
}
```

### The `MyThread` Class

We have to subclass `Thread` to support
1. `run` will call its thread pool's `dispatch` method, and
2. thread cancellation.

Each `MyThread` object is constructed with a `Runnable`, the thread pool it belongs to, and its index in the pool. In its `run` method, we call `dispatch` after the `Runnable` has been executed.

During thread cancellation, each thread's `interrupt()` method is called and the thread marked as completed. This is implemented in the `shutdown` method of `ThreadPool`.

```java
public synchronized void shutdown() {
  for (int i = 0; i < pool.length; ++i)
    if (pool[i] != null) {
      pool[i].interrupt();
      pool[i] = null;
    }
  threadsCount = 0;
}
```

Note that since our implementation eliminates the use of `notify` and `wait`, with contention happening entirely at the object level, and per the API documentation when a thread is waiting in the `wait` method, suspended in the `sleep` method will it throw an `InterruptedException` and return immediately from those functions, no `InterruptedException` will be thrown anywhere in the code. `interrupt()` will merely set the interrupt status and wait for threads to exit. Therefore, the thread itself has to repeatedly check its interrupt status to see whether it should terminate:

```java
public void run() {
  try {
    if (isInterrupted())
      throw new InterruptedException();

    // Execution of the Runnable won't be interrupted.
    super.run();

    if (isInterrupted())
      throw new InterruptedException();

    THREAD_POOL.dispatch(THREAD_POOL_INDEX);
  } catch (InterruptedException e) {
    System.out.println(this + " is interrupted by shutdown(). Exiting...");
  }
}
```

## The Producer-Consumer Problem

Our design is the same as that given in the book: two semaphores for counting the number of empty and full buffers and one mutex lock for mutual exclusive access to the buffer. The buffer is implemented as a circular buffer:

```c
static int insert_item(buffer_item item)
{
    buffer[in] = item;
    in = (in + 1) % BUFFER_SIZE;
    return 0;
}

static int remove_item(buffer_item *item) {
    *item = buffer[out];
    out = (out + 1) % BUFFER_SIZE;
    return 0;
}
```

The semaphore `full` counts the number of full buffers, and `empty` the empty buffers. Each producer is assigned a numeric ID in case that IDs assigned by Pthread of type `pthread_t` may not be numeric.

```c
void *producer(void *p_id)
{
    int id = *(int *)p_id;
    buffer_item item;
    while (true) {
        sleep(rand() % 4);
        item = rand() % 1000;
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);

        if (insert_item(item))
            fprintf(stderr, "report error condition");
        else
            printf("Producer %d produced %d.\n", id, item);

        pthread_mutex_unlock(&mutex);
        sem_post(&full);
    }
    return NULL;
}
```

The `consumer` function is similar to `producer`. A compiled executable is distributed with the handin for demonstration purpose.
