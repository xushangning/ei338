#include "pdeque.h"

int main(int argc, char *argv[]) {
    struct pdeque *d = pdeque_new(atoi(argv[1]));
    int c;
    fputs("allocator> ", stdout);
    while ((c = getchar()) != 'X') {
        if (c == 'R') {
            char process_name[4];
            int size;
            c = getchar();
            if (c == 'Q') {
                char strategy_char;
                enum strategy st;
                scanf(" %3s %d %c", process_name, &size, &strategy_char);
                st = strategy_char == 'F' ? FIRST_FIT
                        : (strategy_char == 'B' ? BEST_FIT : WORST_FIT);
                if (request(d, process_name, size, st))
                    puts("Request granted.");
                else
                    puts("Request denied.");
            }
            else if (c == 'L') {
                scanf(" %3s", process_name);
                release(d, process_name);
            }
        }
        else if (c == 'C')
            compact(d);
        else
            stat(d);

        while (getchar() != '\n');
        fputs("allocator> ", stdout);
    }

    pdeque_free(d);
    return 0;
}
