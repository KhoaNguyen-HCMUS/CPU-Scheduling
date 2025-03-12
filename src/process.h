#pragma once
#include <vector>
#include "Task.h"
using namespace std;

enum State {
  NOT_ARRIVED,
  READY_CPU,
  RUNNING_CPU,
  READY_RES,
  RUNNING_RES,
  FINISHED
};

class Process {
 public:
  int id;              // process number (1-indexed)
  int arrival;         // arrival time to ready queue
  vector<Task> tasks;  // bursts in order: always starting with a CPU burst.
  int curTask;         // index of current burst in tasks
  int remainingTime;   // remaining time for current burst
  State state;         // current state of process
  int finishTime;   // finish time (when the process completes its last burst)
  int waitingTime;  // accumulated waiting time (while in CPU or resource
  // queues)
  int readyCpuTime;
  Process();

  Process& operator=(const Process& p);
};
