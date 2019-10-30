# EI338 Computer System Engineering: Project Report 3

517030910384 徐尚宁

## Introduction

In this project report, we present our implementation of the multi-threaded sorting application, one implemented in C with the Pthread library, the other with Java's fork-join framework.

## Environment

- Arch Linux
- GCC 9.2.0
- OpenJDK 13.0.1

## Multi-threaded Sorting Application

Our multi-threaded sorting program implements the quicksort algorithm. Since quicksort has no "combine" step, our program has one thread for selecting the pivot element and dividing the array, and two threads for sorting the two sublists.

All numbers from input are stored in the global array `a` for easy data sharing. The task for each thread is declared as

```c
void *quicksort(void *left_right);
```

where `left_right` is a two-element array that specifies the range to sort. We use the following code to convert the `void *` pointer to two variables:

```c
int left = ((int *)left_right)[0], right = ((int *)left_right)[1];
```

The selection of pivot is pretty standard for the quicksort algorithm. After elements in the array is arranged according to the pivot, two new threads are created to sort the left and the right half:

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

## Fork-Join Sorting Application

We implement parallel merge sort and quicksort with Java's fork-join concurrency framework. The two sorting algorithms are organized in their respective classes, `MergeSortAction` and `QuicksortAction`, that extend `RecursiveAction` from the framework. Both classes are generic in that they accept any type `T` that implements the `Comparable<T>` interface i.e. any type `T` whose objects can be compared with each other.

Take `QuicksortAction` for example. We override the `compute` method in its superclass to implement pivot selection, partition and running new threads.

```java
protected void compute() {
  int length = end - begin;
  if (length <= 1)
    return;
  else if (length <= THRESHOLD) {
    Main.insertionSort(array, begin, end);
    return;
  }

  var random = new Random();
  int p = begin + random.nextInt(length);
  T pivot = array[p];
  array[p] = array[end - 1];

  int j = begin;
  T temp;
  for (int i = begin; i < end - 1; ++i)
      if (array[i].compareTo(pivot) < 0) {
          temp = array[i];
          array[i] = array[j];
          array[j++] = temp;
      }
  array[end - 1] = array[j];
  array[j] = pivot;

  invokeAll(
    new QuicksortAction<>(array, begin, j),
    new QuicksortAction<>(array, j + 1, end)
  );
}
```

Sorting is delegated to the function `insertionSort` when the size of the subproblem is less than the set `THRESHOLD`, 100 in our case. The code for `MergeSortAction` exhibits a similar structure.

In the `Main` class, we define `insertionSort` and demonstrate `QuickSortAction` and `MergeSortAction`. 1000 random integers are generated for each algorithm for sorting.
