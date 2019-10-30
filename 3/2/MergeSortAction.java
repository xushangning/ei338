import java.util.concurrent.*;
import java.util.Random;
import java.util.Arrays;

public class MergeSortAction<T extends Comparable<T>> extends RecursiveAction {
  private static final int THRESHOLD = 100;

  private int begin;
  private int end;
  private T array[];

  public MergeSortAction(T a[], int b, int e) {
    array = a;
    begin = b;
    end = e;
  }

  protected void compute() {
    int length = end - begin;
    if (length <= 1)
      return;
    else if (length <= THRESHOLD) {
      Main.insertionSort(array, begin, end);
      return;
    }

    int mid = begin + length / 2;
    invokeAll(
      new MergeSortAction<>(array, begin, mid),
      new MergeSortAction<>(array, mid, end)
    );

    var buffer = Arrays.copyOfRange(array, begin, mid);
    for (int i = 0, j = begin, k = mid; i < buffer.length; ++j)
      array[j] = (k == end || buffer[i].compareTo(array[k]) < 0) ? buffer[i++] : array[k++];
  }
}
