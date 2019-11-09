import java.util.List;

public class FCFS implements Algorithm {
  private List<Task> queue;

  public FCFS(List<Task> q) {
    queue = q;
  }

  public void schedule() {
    for (Task t : queue)
      CPU.run(t, t.getBurst());
  }

  /**
   * Unused
   */
  public Task pickNextTask() {
    return new Task("", 0, 0);
  }
}
