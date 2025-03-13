#include "process.h"

Process::Process() {
  id = 0;
  arrival = 0;
  curTask = 0;
  remainingTime = 0;
  state = NOT_ARRIVED;
  finishTime = 0;
  waitingTime = 0;
  readyCpuTime = -1;
}

Process& Process::operator=(const Process& p) {
  if (this == &p) return *this;
  id = p.id;
  arrival = p.arrival;
  tasks = p.tasks;
  curTask = p.curTask;
  remainingTime = p.remainingTime;
  state = p.state;
  finishTime = p.finishTime;
  waitingTime = p.waitingTime;
  readyCpuTime = p.readyCpuTime;
  return *this;
}
