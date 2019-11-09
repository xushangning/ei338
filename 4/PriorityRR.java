import java.util.*;

public class PriorityRR implements Algorithm {
  private static final int TIME_QUANTUM = 10;

  private class DescIntComp implements Comparator<Integer> {
    public int compare(Integer i1, Integer i2) {
      if (i1 == i2)
        return 0;
      return i1 > i2 ? -1 : 1;
    }
  }

  private SortedMap<Integer, List<Task>> queues;

  public PriorityRR(List<Task> q) {
    queues = new TreeMap<Integer, List<Task>>(new DescIntComp());
    for (Task t : q) {
      int priority = t.getPriority();
      List<Task> queue = queues.get(priority);
      if (queue == null) {
        var newQueue = new ArrayList<Task>();
        newQueue.add(t);
        queues.put(priority, newQueue);
      }
      else
        queue.add(t);
    }
  }

  public void schedule() {
    for (int k : queues.keySet()) {
      List<Task> queue = queues.get(k);
      int i = 0;
      do {
        Task t = queue.get(i);
        int burst = t.getBurst();
        if (burst > TIME_QUANTUM) {
          CPU.run(t, TIME_QUANTUM);
          t.setBurst(burst - TIME_QUANTUM);
          i = (i + 1) % queue.size();
        }
        else {
          CPU.run(t, burst);
          queue.remove(i);
        }
      } while (!queue.isEmpty());
    }
  }

  /**
   * Unused
   */
  public Task pickNextTask() { return new Task("", 0, 0); }
}
