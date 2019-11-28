#include "hardware.h"

#include <assert.h>

int main() {
    unsigned logical_addr, physical_addr;
    int correct, test;
    FILE *f_addr = fopen("addresses.txt", "r");
    FILE *f_correct = fopen("correct.txt", "r");
    while (fscanf(f_addr, "%d", &logical_addr) == 1) {
        physical_addr = translate(logical_addr);
        test = (int8_t) memory[physical_addr];
        fscanf(f_correct, "%d", &correct);
        assert(test == correct);
        printf("%d\n", test);
    }
    printf("TLB Hit Rate: %lf\n", (double) tlb.hits / tlb.accesses);
    printf("Page Fault Rate: %lf\n",
            (double) page_table.faults / page_table.accesses);

    return 0;
}
