# EI338 Computer System Engineering: Project Report 6

517030910384 徐尚宁

## Introduction

In this project report, we present our implementation of the banker algorithm.

## Environment

- Arch Linux
- GCC 9.2.0

## Implementation

The command `*` is implemented in the function `print_states`.

We define `request_resources()` for resource request and `release_resources()` for resource release. `release_resource()` just takes the customer number and a list of resources, and update the `available`, `need` and `allocation` arrays.

The parameters for `request_resource` are the customer number and the list of resources requested. First, it checks whether there are sufficient available resources to satisfy the request. What follows is the banker algorithm that checks whether granting the request will leave the system in an unsafe state.

As what has been illustrated in the textbook, an array `finish` is declared. We loop over the `finish` array in a circular fashion. 

At the start of the loop body, `finish[i]` is checked. If `finish[i]` is `false`, we will try to do a mock allocation with all available resources. If customer `i` potential demand can't be satisfied, `cant_allocate` will be set to `false`.

Because we treated the array as circular, the variable `count` is dedicated to counting whether we have loop over all the customers since last allocation in the banker algorithm. At the loop's exit, we have looped over all the customers since last allocation and if `cant_allocate`  is `false`, there is still one customer whose potential demand for resources can't be satisfied, so the request has to be denied. Otherwise, the request can granted.

```c
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
```

## Demo

The following resource-allocation situation is adapted from Exercise 8.9.

```
$ cmake-build-debug/6 12 13 6 8
> RQ 0 3 0 1 4
Request granted.
> RQ 1 2 2 1 0
Request granted.
> RQ 2 3 1 2 1
Request granted.
> RQ 3 0 5 1 0
Request granted.
> RQ 4 4 2 1 2
Request denied.
> *
available = 4 5 1 3 
maximum = 
    5 1 1 7 
    3 2 1 1 
    3 3 2 1 
    4 6 1 2 
    6 3 2 5 
allocation = 
    3 0 1 4 
    2 2 1 0 
    3 1 2 1 
    0 5 1 0 
    0 0 0 0 
need = 
    2 1 0 3 
    1 0 0 1 
    0 2 0 0 
    4 1 0 2 
    6 3 2 5 
> RL 3 0 5 1 0
> RQ 4 4 2 1 2
Request denied.
> *
available = 4 10 2 3 
maximum = 
    5 1 1 7 
    3 2 1 1 
    3 3 2 1 
    4 6 1 2 
    6 3 2 5 
allocation = 
    3 0 1 4 
    2 2 1 0 
    3 1 2 1 
    0 0 0 0 
    0 0 0 0 
need = 
    2 1 0 3 
    1 0 0 1 
    0 2 0 0 
    4 6 1 2 
    6 3 2 5 
```
