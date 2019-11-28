# EI338 Computer System Engineering: Project Report 8

517030910384 徐尚宁

## Introduction

In this project report, we present our implementation of the virtual memory manager.

## Environment

- Arch Linux
- GCC 9.2.0

## Implementation

The translation process involve two functions: `translate` and `page_table_translate`, which are responsible for accessing and updating the TLB and the page table, respectively. Three data structures `tlb`, `page_table` and `frame_table` supports the address translation translation. A large array `memory` emulates the main memory.

The virtual memory manager is designed with the assumption that the memory size is less than the backing store at its inception, so the main memory has 128 pages while the backing store has 256 pages.

### TLB

We use bit fields in C to precisely specify the storage size for each field. Each TLB entry consists of an 8-bit page number, 7-bit frame number and a 1-bit valid/invalid flag.

```c
struct tlb_entry {
    unsigned pn: 8, fn: 7;
    bool valid: 1;
};

struct {
    struct tlb_entry table[TLB_SIZE];
    int accesses, hits;
} tlb;
```

Accessing the TLB in `translate()` begins with reading each entry to find a matching page number. If the access results in a miss, the page number is passed to `page_table_translate` and the translated physical address is written to the TLB. The random replacement policy is used.

### Page Table

Each page table entry (represented by the struct `page_table_entry`) contains a frame number and a valid/invalid bit.

The free-frame list is implemented with a circular array named `frame_table` to support the FIFO replacement policy. The pointer `next_insert_pos` points to the next frame to be replaced, which may be free or occupied. `frame_table.table[i] == j` means that the `i`th frame is mapped to the `j`th page in the backing store, which also happens to be the `j`th page in the process's logical address space. In this way, `frame_table` can be considered as the inverse mapping of the page table.

```c
struct page_table_entry {
    unsigned n: 7;
    bool valid: 1;
};

/**
 * The page table. page_table.table[i] == j means that page number i translates
 * to physical address/frame number j.
 */
struct {
    struct page_table_entry table[PAGE_TABLE_SIZE];
    int accesses, faults;
} page_table;

/**
 * Reverse lookup table. frame_table.table[i] == j means that the ith frame is
 * mapped to the jth page in the backing store.
 */
struct {
    uint8_t table[FRAME_TABLE_SIZE];
    int next_insert_pos;
} frame_table;
```

In `page_translate`, a page fault happens when the page table entry is invalid. A new frame will be allocated from the main memory, and the page table and TLB are updated accordingly. The page will be read from the backing store to the main memory. The correct physical address will be returned.

## Demo

In the demo below, address translation output is omitted, as it matches the file `correct.txt` exactly. Only the statistics is shown.

```
TLB Hit Rate: 0.066000
Page Fault Rate: 0.576017
```

The low TLB hit rate and around-half page fault rate are consistent with our prediction, due to the nature of randomly-generated memory accesses and the size ratio between the main memory and the backing store.
