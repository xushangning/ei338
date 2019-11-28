#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define MEMORY_SIZE 32768
#define PAGE_SIZE 256
#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
#define FRAME_TABLE_SIZE 128

uint8_t memory[MEMORY_SIZE];

struct tlb_entry {
    unsigned pn: 8, fn: 7;
    bool valid: 1;
};

struct {
    struct tlb_entry table[TLB_SIZE];
    int accesses, hits;
} tlb;

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

unsigned translate(unsigned logical_addr);
unsigned page_table_translate(unsigned page_number);

#endif // HARDWARE_H
