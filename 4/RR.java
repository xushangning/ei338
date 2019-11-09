import java.util.List;

public class RR implements Algorithm {
  static final private int TIME_QUANTUM = 10;

  private List<Task> queue;
  private int lastTaskIndex;

  public RR(List<Task> q) {
    queue = q;
    lastTaskIndex = -1;   // set to 0 at first call to pickNextTask
  }

  public void schedule() {
    while (!queue.isEmpty()) {
      Task t = pickNextTask();
      int burst = t.getBurst();
      if (burst > TIME_QUANTUM) {
        CPU.run(t, TIME_QUANTUM);
        t.setBurst(burst - TIME_QUANTUM);
      }
      else {
        CPU.run(t, burst);
        // next call to pickNextTask will increment pickNextTask
        queue.remove(lastTaskIndex--);
      }
    }
  }

  public Task pickNextTask() {
    lastTaskIndex = (lastTaskIndex + 1) % queue.size();
    return queue.get(lastTaskIndex);
  }
}
