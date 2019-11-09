import java.util.List;
import java.util.Comparator;

public class Priority implements Algorithm {
  private class PriorityComp implements Comparator<Task> {
    public int compare(Task t1, Task t2) {
      int p1 = t1.getPriority(), p2 = t2.getPriority();
      if (p1 == p2)
        return 0;
      return p1 > p2 ? -1 : 1;
    }
  }

  private List<Task> queue;

  public Priority(List<Task> q) {
    queue = q;
    queue.sort(new PriorityComp());
  }

  public void schedule() {
    while (!queue.isEmpty()) {
      Task t = pickNextTask();
      CPU.run(t, t.getBurst());
    }
  }

  public Task pickNextTask() {
    Task t = queue.get(0);
    queue.remove(0);
    return t;
  }
}
