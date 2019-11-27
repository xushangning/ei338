#include "hardware.h"

#include <stdlib.h>

unsigned translate(unsigned logical_addr)
{
    unsigned offset = logical_addr & 0xFF,
        page_number = (logical_addr >> 8) & 0xFF, frame_number;
    bool not_found = true;
    struct tlb_entry *te;

    for (int i = 0; i < TLB_SIZE; ++i) {
        te = tlb + i;
        if (te->valid && te->pn == page_number) {
            frame_number = te->fn;
            not_found = false;
            break;
        }
    }

    if (not_found) {
        frame_number = page_table_translate(page_number);
        not_found = true;
        for (int i = 0; i < TLB_SIZE; ++i) {
            te = tlb + i;
            if (!te->valid) {
                not_found = false;
                break;
            }
        }
        if (not_found)
            te = tlb + rand() % TLB_SIZE;
        te->pn = page_number;
        te->fn = frame_number;
        te->valid = true;
    }

    return (frame_number << 8) | offset;
}

unsigned page_table_translate(unsigned page_number)
{
    struct page_table_entry *pte = page_table + page_number;
    if (pte->valid)
        return pte->n;

    // FIFO implemented with a circular array
    int insert_pos = frame_table.next_insert_pos;
    frame_table.next_insert_pos = (insert_pos + 1) % FRAME_TABLE_SIZE;

    uint8_t override_pn = frame_table.table[insert_pos];
    frame_table.table[insert_pos] = page_number;
    struct page_table_entry *override = page_table + override_pn;

    // The frame table is initialized to all zeroes when created, leading to
    // the illusion that all frames are allocated for page 0 in the backing
    // store. Therefore, we need to check whether the two mappings (page_table
    // and frame_table) are equivalent.
    //
    // If override->n != insert_pos, then this is the first allocation for
    // the (insert_pos)th frame.
    if (override->n == (unsigned) insert_pos && override->valid) {
        // mark TLB entry as invalid
        for (int i = 0; i < TLB_SIZE; ++i) {
            struct tlb_entry *te = tlb + i;
            if (te->pn == override_pn && te->valid) {
                te->valid = false;
                break;
            }
        }
    }
    // mark page table entry as invalid
    override->valid = false;
    page_table[page_number].n = insert_pos;
    page_table[page_number].valid = true;

    // move data from the backing store to the main memory
    FILE *bs = fopen("BACKING_STORE.bin", "rb");
    fseek(bs, page_number * PAGE_SIZE, SEEK_SET);
    fread(memory + insert_pos * PAGE_SIZE, 1, PAGE_SIZE, bs);

    return insert_pos;
}
