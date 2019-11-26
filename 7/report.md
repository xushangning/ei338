# EI338 Computer System Engineering: Project Report 7

517030910384 徐尚宁

## Introduction

In this project report, we present our implementation of the contiguous memory allocator.

## Environment

- Arch Linux
- GCC 9.2.0

## Implementation

### The Process Deque `pdeque`

A doubly-linked list (deque) is used to maintain the state of memory, which includes each process's name, start address and end address. This deque is called `pdeque`, for "process deque".

Each node in the deque, the `pdeque_node`, represents a process. Unused memory is implicitly represented in the difference between a process's end address and its successor's start address. A `pdeque` struct contains a pointer to a `pdeque_node` that is the head of the deque.

The following functions are declared to manipulate `pdeque`:

```c
// deque operations
struct pdeque *pdeque_new(int size);
void pdeque_node_free(struct pdeque_node *n);
void pdeque_free(struct pdeque *d);
void pdeque_insert_after(struct pdeque_node *p, struct pdeque_node *new);
void pdeque_prepend(struct pdeque *d, struct pdeque_node *new);

// operations for contiguous memory allocation
bool request(struct pdeque *d, const char *name, int size, enum strategy st);
void release(struct pdeque *d, const char *name);
void compact(struct pdeque *d);
void stat(struct pdeque *d);
```

The first 5 functions are pretty standard deque operations, so they are not elaborated here.

### Memory Allocation

To request memory allocation, the function `request` is called with the process name, required memory size and allocation strategy. During allocation, the `request` function will iterate over holes between neighboring processes and select the optimal hole according to the strategy.

If a usable hole is found, that hole is represented by the `pdeque_node` before it so that we can just insert our new process after that `pdeque_node` in the deque. A new `pdeque_node` will be created and inserted into the deque. The case where the node is inserted at the head is handled separately.

A boolean value is returned that indicates success or failure.

In the `release` function, we try to find a `pdeque_node` with the same process name as the one given at the command line and remove it from the deque.

The `compact` function is implemented simply as changing the start and end addresses of each `pdeque_node`.

## Demo

In the demo below, first holes of various sizes are created, and then each allocation strategy is tested.

```
$ ./allocator 1048576
allocator> RQ P2 20000 F
Request granted.
allocator> RQ P1 40000 F
Request granted.
allocator> RQ P0 10000 F
Request granted.
allocator> RQ P3 40000 F
Request granted.
allocator> RQ P4 30000 F
Request granted.
allocator> RQ P5 40000 F
Request granted.
allocator> RL P0
allocator> RL P2
allocator> RL P4
allocator> STAT
Addresses [0:20000] Unused
Addresses [20000:60000] P1
Addresses [60000:70000] Unused
Addresses [70000:110000] P3
Addresses [110000:140000] Unused
Addresses [140000:180000] P5
Addresses [180000:1048576] Unused
allocator> RQ P0 9000 F
Request granted.
allocator> STAT
Addresses [0:9000] P0
Addresses [9000:20000] Unused
Addresses [20000:60000] P1
Addresses [60000:70000] Unused
Addresses [70000:110000] P3
Addresses [110000:140000] Unused
Addresses [140000:180000] P5
Addresses [180000:1048576] Unused
allocator> RL P0
allocator> RQ P0 9000 B
Request granted.
allocator> STAT
Addresses [0:20000] Unused
Addresses [20000:60000] P1
Addresses [60000:69000] P0
Addresses [69000:70000] Unused
Addresses [70000:110000] P3
Addresses [110000:140000] Unused
Addresses [140000:180000] P5
Addresses [180000:1048576] Unused
allocator> RL P0 
allocator> RQ P0 9000 W
Request granted.
allocator> STAT
Addresses [0:20000] Unused
Addresses [20000:60000] P1
Addresses [60000:69000] P0
Addresses [69000:70000] Unused
Addresses [70000:110000] P3
Addresses [110000:140000] Unused
Addresses [140000:180000] P5
Addresses [180000:189000] P0
Addresses [189000:1048576] Unused
allocator> C
allocator> STAT
Addresses [0:40000] P1
Addresses [40000:49000] P0
Addresses [49000:89000] P3
Addresses [89000:129000] P5
Addresses [129000:138000] P0
Addresses [138000:1048576] Unused
allocator> X
```
