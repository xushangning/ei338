import java.util.concurrent.*;
import java.util.Random;

public class QuicksortAction<T extends Comparable<T>> extends RecursiveAction {
  private static final int THRESHOLD = 100;

  private int begin;
  private int end;
  private T array[];

  public QuicksortAction(T a[], int b, int e) {
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
}
