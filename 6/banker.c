#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 4
#define MAX_DEMAND_INPUT_FILE "max-demand.txt"

/* the available amount of each resource */
int available[NUMBER_OF_RESOURCES];

/* the maximum demand of each customer */
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

/* the amount currently allocated to each customer */
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

/* the remaining need of each customer */
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

int request_resources(int customer_num, const int request[]);
void release_resources(int customer_num, const int release[]);

void print_states(void)
{
    fputs("available = ", stdout);
    for (int i = 0; i < NUMBER_OF_RESOURCES; ++i)
        printf("%d ", available[i]);
    putchar('\n');

    puts("maximum = ");
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; ++i) {
        fputs("    ", stdout);
        for (int j = 0; j < NUMBER_OF_RESOURCES; ++j)
            printf("%d ", maximum[i][j]);
        putchar('\n');
    }

    puts("allocation = ");
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; ++i) {
        fputs("    ", stdout);
        for (int j = 0; j < NUMBER_OF_RESOURCES; ++j)
            printf("%d ", allocation[i][j]);
        putchar('\n');
    }

    puts("need = ");
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; ++i) {
        fputs("    ", stdout);
        for (int j = 0; j < NUMBER_OF_RESOURCES; ++j)
            printf("%d ", need[i][j]);
        putchar('\n');
    }
}

/**
 * Initialize the array available and maximum
 */
void init_states(char *argv[])
{
    FILE *fp = fopen(MAX_DEMAND_INPUT_FILE, "r");
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; ++i) {
        // manually read the first number so that later numbers can be read
        // with the format string ",%d"
        fscanf(fp, "%d", &maximum[i][0]);
        need[i][0] = maximum[i][0];
        for (int j = 1; j < NUMBER_OF_RESOURCES; ++j) {
            fscanf(fp, ",%d", &maximum[i][j]);
            need[i][j] = maximum[i][j];
        }
    }

    for (int i = 0; i < NUMBER_OF_RESOURCES; ++i)
        available[i] = atoi(argv[i + 1]);
}

int main(int argc, char *argv[])
{
    init_states(argv);

    char command[3];
    fputs("> ", stdout);
    while (fgets(command, 3, stdin)) {
        if (!strcmp(command, "*\n")) {
            print_states();
            ungetc('\n', stdin);
        }
        else if (!strcmp(command, "RQ")) {
            int customer, request[NUMBER_OF_RESOURCES], retval;
            scanf("%d", &customer);
            for (int i = 0; i < NUMBER_OF_RESOURCES; ++i)
                scanf("%d", &request[i]);
            if ((retval = request_resources(customer, request)) == 0)
                puts("Request granted.");
            else if (retval == -1)
                puts("Request denied.");
        }
        else if (!strcmp(command, "RL")) {
            int customer, release[NUMBER_OF_RESOURCES];
            scanf("%d", &customer);
            for (int i = 0; i < NUMBER_OF_RESOURCES; ++i)
                scanf("%d", &release[i]);
            release_resources(customer, release);
        }

        while (getchar() != '\n');

        fputs("> ", stdout);
    }
    return 0;
}

int request_resources(int customer_num, const int request[])
{
    for (int i = 0; i < NUMBER_OF_RESOURCES; ++i)
        if (available[i] < request[i])
            return -1;

    int temp_available[NUMBER_OF_RESOURCES];
    for (int i = 0; i < NUMBER_OF_RESOURCES; ++i) {
        need[customer_num][i] -= request[i];
        allocation[customer_num][i] += request[i];
        temp_available[i] = available[i] - request[i];
    }

    bool finish[NUMBER_OF_CUSTOMERS] = {false};
    bool cant_allocate = true, can_complete = true;
    int count = 0;

    for (int i = 0; count < NUMBER_OF_CUSTOMERS; i = (i + 1) % NUMBER_OF_CUSTOMERS) {
        if (finish[i])
            ++count;
        else {
            for (int j = 0; j < NUMBER_OF_RESOURCES; ++j)
                if (temp_available[j] < need[i][j]) {
                    can_complete = false;
                    break;
                }

            cant_allocate = !can_complete;
            if (can_complete) {
                finish[i] = true;
                for (int j = 0; j < NUMBER_OF_RESOURCES; ++j)
                    temp_available[j] += allocation[i][j];
                count = 0;
            }
            else
                ++count;
        }
    }

    if (cant_allocate) {
        for (int i = 0; i < NUMBER_OF_RESOURCES; ++i) {
            need[customer_num][i] += request[i];
            allocation[customer_num][i] -= request[i];
        }
        return -1;
    }

    for (int i = 0; i < NUMBER_OF_RESOURCES; ++i)
        available[i] -= request[i];
    return 0;
}

void release_resources(int customer_num, const int release[])
{
    for (int i = 0; i < NUMBER_OF_RESOURCES; ++i) {
        available[i] += release[i];
        need[customer_num][i] += release[i];
        allocation[customer_num][i] -= release[i];
    }
}
