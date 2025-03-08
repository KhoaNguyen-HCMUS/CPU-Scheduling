#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <deque>

#include "process.h"

using namespace std;

class Scheduler {
 private:
  int algorithm;
  int quantum;
  int numProc;
  vector<Process> procList;

  int time;
  int finishedCount;

  int runningCPU;      // index of process running on CPU (-1 means idle)
  int currentQuantum;  // for Round Robin

  deque<int> cpuQueue;   // indices of processes ready for CPU
  deque<int> resQueue1;  // resource R1 waiting queue
  deque<int> resQueue2;  // resource R2 waiting queue

  // Gantt chart records (each time unit one entry)
  vector<string> cpuTimeline;
  vector<string> resTimeline1;
  vector<string> resTimeline2;

  // Resource usage: which process (index) is currently using the resource
  int runningRes1;
  int runningRes2;

  // Vector to hold processes that finished CPU burst and should start their R
  // burst in the next step.
  vector<int> pendingResource;
  int lastCpuBusy;

 public:
  Scheduler();

  void readInput(string inputFileName);
  void schedule();
  void writeOutput(string outputFileName);
};
