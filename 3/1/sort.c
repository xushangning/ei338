#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int a[100];

void *quicksort(void *left_right);

int main(void)
{
    int n;
    scanf("%d", &n);
    for (int i = 0; i < n; ++i)
        scanf("%d", &a[i]);

    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int left_right[2] = {0, n};
    pthread_create(&tid, &attr, quicksort, left_right);
    pthread_join(tid, NULL);

    for (int i = 0; i < n - 1; ++i)
        printf("%d ", a[i]);
    printf("%d\n", a[n - 1]);

    return 0;
}

void *quicksort(void *left_right)
{
    int left = ((int *)left_right)[0], right = ((int *)left_right)[1];
    if (left + 1 >= right)
        return NULL;
    if (left + 2 == right) {
        if (a[left] > a[left + 1]) {
            int temp = a[left];
            a[left] = a[left + 1];
            a[left + 1] = temp;
        }
        return NULL;
    }

    int size = right - left, p = left + random() % size;
    int temp = a[p], pivot = temp;
    a[p] = a[right - 1];
    int j;
    for (p = j = left; j < right - 1; ++j)
        if (a[j] < pivot) {
            temp = a[j];
            a[j] = a[p];
            a[p++] = temp;
        }
    a[right - 1] = a[p];
    a[p] = pivot;

    pthread_t tid[2];
    pthread_attr_t attr[2];
    int params[4] = {left, p, p + 1, right};
    pthread_attr_init(attr);
    pthread_create(tid, attr, quicksort, params);
    pthread_attr_init(attr);
    pthread_create(tid + 1, attr + 1, quicksort, params + 2);
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    return NULL;
}
