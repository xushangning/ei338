import java.util.LinkedList;

/**
 * A simple thread pool API.
 * 
 * Tasks that wish to get run by the thread pool must implement the
 * java.lang.Runnable interface.
 */

public class ThreadPool {
  static private final int DEFAULT_SIZE = 3;

  private MyThread pool[];
  private LinkedList<Runnable> queue;
  private int threadsCount;

  /**
   * Create a default size thread pool.
   */
  public ThreadPool() {
    pool = new MyThread[DEFAULT_SIZE];
    queue = new LinkedList<Runnable>();
  }
  
  
  /**
   * Create a thread pool with a specified size.
   * 
   * @param int size The number of threads in the pool.
   */
  public ThreadPool(int size) {
    pool = new MyThread[size];
    queue = new LinkedList<Runnable>();
  }
  
  
  /**
   * shut down the pool.
   */
  public synchronized void shutdown() {
    for (int i = 0; i < pool.length; ++i)
      if (pool[i] != null) {
        pool[i].interrupt();
        pool[i] = null;
      }
    threadsCount = 0;
  }
  
  /**
   * Add work to the queue.
   */
  public synchronized void add(Runnable task) {
    if (threadsCount < pool.length)
      for (int i = 0; i < pool.length; ++i)
        if (pool[i] == null) {
          if (!queue.isEmpty()) {
            queue.addLast(task);
            task = queue.removeFirst();
          }
          pool[i] = new MyThread(task, this, i);
          pool[i].start();
          ++threadsCount;

          break;
        }
    else
      queue.addLast(task);
  }

  /**
   * Mark the thread pool[threadPoolIndex] as completed and start a new thread
   * to execute a task if there is still work to do.
   */
  public synchronized void dispatch(int threadPoolIndex) {
    if (!queue.isEmpty()) {
      Runnable t = queue.removeFirst();
      pool[threadPoolIndex] = new MyThread(t, this, threadPoolIndex);
      pool[threadPoolIndex].start();
    }
    else {
      // mark completed threads as null
      pool[threadPoolIndex] = null;
      --threadsCount;
    }
  }
}
