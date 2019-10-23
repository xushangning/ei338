# EI338 Computer System Engineering: Project Report 3

517030910384 徐尚宁

## Introduction

In this project report, we present our implementation of the multi-threaded sorting application.

## Multi-threaded Sorting Application

Our multi-threaded sorting program implements the quicksort algorithm. Since quicksort has no "combine" step, our program has one thread for selecting the pivot element and dividing the array, and two threads for sorting the two sublists.

All numbers from input are stored in the global array `a` for easy data sharing. The task for each thread is declared as

```c
void *quicksort(void *left_right);
```

where `left_right` is a two-element array that specifies the range to sort. The selection of pivot is pretty standard for the quicksort algorithm. After elements in the array is arranged according to the pivot, two new threads are created to sort the left and the right half:

```c
pthread_t tid[2];
pthread_attr_t attr[2];
int params[4] = {left, p, p + 1, right};
pthread_attr_init(attr);
pthread_create(tid, attr, quicksort, params);
pthread_attr_init(attr);
pthread_create(tid + 1, attr + 1, quicksort, params + 2);
pthread_join(tid[0], NULL);
pthread_join(tid[1], NULL);
```

In the `main` function, we just read all the input numbers and fork a new thread to run the function `quicksort`.
