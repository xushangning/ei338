import java.util.concurrent.ForkJoinPool;
import java.util.Random;

class Main {
  public static <T extends Comparable<T>> void insertionSort(T a[], int begin, int end) {
    T x;
    for (int i = begin + 1; i < end; ++i) {
      x = a[i];
      int j;
      for (j = i - 1; j >= 0; --j)
        if (x.compareTo(a[j]) < 0)
          a[j + 1] = a[j];
        else
          break;
      a[j + 1] = x;
    }
  }

  public static void main(String args[]) {
    var a = new Integer [2000];
    var random = new Random();
    for (int i = 0; i < a.length; ++i)
        a[i] = random.nextInt();

    var fjp = new ForkJoinPool();
    fjp.invoke(new QuicksortAction<>(a, 0, a.length));

    for (int x : a)
        System.out.print(x + " ");
    System.out.println();
  }
}
