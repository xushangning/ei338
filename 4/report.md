# EI338 Computer System Engineering: Project Report 4

517030910384 徐尚宁

## Introduction

In this project report, we implement and simulate various scheduling algorithms in our simulation environment, written in Java.

## Environment

- Arch Linux
- OpenJDK 13.0.1

## Scheduling Algorithms

Some of the algorithms can be implemented in a pretty straight forward fashion:
- First-come-first-serve: Just go through the task list and dispatch each task.
- RR: Treat the task list as a circular list, dispatch each task with a time quantum of 10 (or less if its burst is less than 10) until the list is empty.

This report focus on the rest of the algorithms, that is, shortest-job-first, priority and priority with round robin.

### SJF and RR

SJF is actually RR with each task's CPU burst as its priority (a larger number indicates lower priority). Because all tasks arrive at the same time, we can simply sort the task list according to the priorities or CPU bursts in descending order and go through the task list:

```java
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
```

The task list is sorted at `Priority`'s construction.

Or we go through the list each time `pickNextTask()` is called, as is evident in our implementation of SJF:

```java
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
```

### Priority with RR

Our implementation of the priority-with-RR algorithm has one task queue for every priority, which is made possible by the use of `SortedMap` that associates each priority with a `List<Task>`.

In the constructor of `PriorityRR`, each task in the original queue is moved to a new queue in the `SortedMap` object:

```java
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
```

Keys in `SortedMap`, as the name implies, are sorted. We loop through the set of keys and process the tasks in each queue in a round-robin fashion:

```java
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
```

We feel that the use of `pickNextTask()` is unnecessary and even troublesome in our implementation, so the role of `pickNextTask()` is integrated into `schedule()`.
