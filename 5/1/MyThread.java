class MyThread extends Thread {
  private final ThreadPool THREAD_POOL;
  private final int THREAD_POOL_INDEX;

  MyThread(Runnable target, ThreadPool tp, int tpi) {
    super(target);
    THREAD_POOL = tp;
    THREAD_POOL_INDEX = tpi;
  }

  public void run() {
    try {
      if (isInterrupted())
        throw new InterruptedException();

      // Execution of the Runnable won't be interrupted.
      super.run();

      if (isInterrupted())
        throw new InterruptedException();

      THREAD_POOL.dispatch(THREAD_POOL_INDEX);
    } catch (InterruptedException e) {
      System.out.println(this + " is interrupted by shutdown(). Exiting...");
    }
  }
}
