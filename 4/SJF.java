import java.util.List;

public class SJF implements Algorithm {
  private List<Task> queue;

  public SJF(List<Task> q) { queue = q; }

  public void schedule() {
    while (!queue.isEmpty()) {
      Task t = pickNextTask();
      CPU.run(t, t.getBurst());
    }
  }

  public Task pickNextTask() {
    int minBurst = Integer.MAX_VALUE, minBurstTaskIndex = 0, i = 0;
    Task minBurstTask = null;
    for (Task t : queue) {
      if (minBurst > t.getBurst()) {
        minBurst = t.getBurst();
        minBurstTask = t;
        minBurstTaskIndex = i;
      }
      ++i;
    }
    queue.remove(minBurstTaskIndex);
    return minBurstTask;
  }
}
