#include "scheduler.h"
#include <algorithm>  // min_element

Scheduler::Scheduler() {
  algorithm = 0;
  quantum = 0;
  numProc = 0;
  time = 0;
  finishedCount = 0;
  runningCPU = -1;
  currentQuantum = 0;

  runningRes1 = -1;
  runningRes2 = -1;
  lastCpuBusy = -1;
}

void Scheduler::readInput(string inputFileName) {
  ifstream fin(inputFileName);
  if (!fin) {
    cerr << "Cannot open input file." << endl;
    exit(1);
  }

  fin >> algorithm;
  if (algorithm == 2) {
    fin >> quantum;
    fin >> numProc;
  } else {
    fin >> numProc;
  }
  procList.resize(numProc);

  string dummy;
  getline(fin, dummy);  // consume rest of line
  std::map<std::string, int> resourceMap;
  // Parse each process’s description.
  for (int i = 0; i < numProc; i++) {
    Process p;
    p.id = i + 1;
    p.curTask = 0;
    p.waitingTime = 0;
    p.state = NOT_ARRIVED;

    string line;
    getline(fin, line);
    if (line.empty()) {
      i--;
      continue;
    }
    istringstream iss(line);
    iss >> p.arrival;
    string token;
    // The rest tokens represent bursts.

    // Add this at the start of main()
    while (iss >> token) {
      Task t;
      // Check if token is a resource burst (contains a '(')
      if (token.find('(') != string::npos) {
        size_t pos = token.find('(');
        size_t pos2 = token.find(')');

        if (pos2 == string::npos) {
          throw std::runtime_error("Missing closing parenthesis");
        }

        string timeStr = token.substr(0, pos);
        string resStr = token.substr(pos + 1, pos2 - pos - 1);
        t.time = stoi(timeStr);
        t.isCPU = false;

        // Modified resource ID assignment
        if (resourceMap.find(resStr) == resourceMap.end()) {
          // New resource found
          int newId = resourceMap.size() + 1;
          if (newId > 2) {
            throw std::runtime_error(
                "Too many different resources (maximum is 2)");
          }
          resourceMap[resStr] = newId;
          t.resourceId = newId;

        } else {
          // Existing resource
          t.resourceId = resourceMap[resStr];
        }
      } else {
        // CPU burst
        t.time = stoi(token);
        t.isCPU = true;
      }
      p.tasks.push_back(t);
    }

    if (!p.tasks.empty() && p.tasks[0].isCPU)
      p.remainingTime = p.tasks[0].time;
    else
      p.remainingTime = 0;

    procList[i] = p;
  }

  fin.close();
}

void Scheduler::schedule() {
  // Main simulation loop (step by step until all processes are finished)

  while (true) {
    // At the beginning of the simulation step, add pending resource processes
    // to resource queues.
    if (!pendingResource.empty()) {
      for (int idx : pendingResource) {
        int resId = procList[idx].tasks[procList[idx].curTask].resourceId;
        procList[idx].state = READY_RES;
        if (resId == 1)
          resQueue1.push_back(idx);
        else
          resQueue2.push_back(idx);
      }
      pendingResource.clear();
    }
    // Termination: if all processes are finished and no process is in any
    // queue or running
    if (finishedCount == numProc && runningCPU == -1 && cpuQueue.empty() &&
        runningRes1 == -1 && runningRes2 == -1 && resQueue1.empty() &&
        resQueue2.empty())
      break;
    // (1) Check for arrivals at current time.

    for (int i = 0; i < numProc; i++) {
      if (procList[i].state == NOT_ARRIVED && procList[i].arrival == time) {
        procList[i].state = READY_CPU;
        cpuQueue.push_back(i);
      }
    }

    // (2) CPU Scheduling:
    // If no process is running on CPU, choose one from the ready queue.
    if (runningCPU == -1 && !cpuQueue.empty()) {
      int idx = -1;
      if (algorithm == 1 || algorithm == 2) {
        // FCFS or Round Robin: select the first process in the queue.
        idx = cpuQueue.front();
        cpuQueue.pop_front();
      } else if (algorithm == 3) {
        // SJF: choose the process with the smallest remaining CPU burst.
        auto it =
            min_element(cpuQueue.begin(), cpuQueue.end(), [&](int a, int b) {
              return procList[a].remainingTime < procList[b].remainingTime;
            });
        idx = *it;
        cpuQueue.erase(it);
      } else if (algorithm == 4) {
        // SRTN: choose process with smallest remaining time.
        auto it =
            min_element(cpuQueue.begin(), cpuQueue.end(), [&](int a, int b) {
              return procList[a].remainingTime < procList[b].remainingTime;
            });
        idx = *it;
        cpuQueue.erase(it);
      }
      runningCPU = idx;
      procList[idx].state = RUNNING_CPU;

      // For Round Robin, reset the quantum.
      if (algorithm == 2) currentQuantum = quantum;
    }

    // For SRTN, check if a process in cpuQueue has a shorter remaining burst
    // than the one on CPU.
    if (runningCPU != -1 && algorithm == 4 && !cpuQueue.empty()) {
      auto it =
          min_element(cpuQueue.begin(), cpuQueue.end(), [&](int a, int b) {
            return procList[a].remainingTime < procList[b].remainingTime;
          });
      int candidate = *it;
      if (procList[candidate].remainingTime <
          procList[runningCPU].remainingTime) {
        // Preempt the running process.
        procList[runningCPU].state = READY_CPU;
        cpuQueue.push_back(runningCPU);
        runningCPU = candidate;
        cpuQueue.erase(it);
        procList[runningCPU].state = RUNNING_CPU;
      }
    }

    // (3) CPU burst processing.
    if (runningCPU != -1) {
      procList[runningCPU].remainingTime--;
      cpuTimeline.push_back(to_string(procList[runningCPU].id));
      if (procList[runningCPU].remainingTime == 0) {
        procList[runningCPU].curTask++;
        if (procList[runningCPU].curTask < procList[runningCPU].tasks.size() &&
            !procList[runningCPU].tasks[procList[runningCPU].curTask].isCPU) {
          // Next burst is a resource burst: add to pendingResource.
          pendingResource.push_back(runningCPU);
        } else {
          // No further task: process is finished.
          procList[runningCPU].state = FINISHED;
          procList[runningCPU].finishTime = time + 1;
          finishedCount++;
        }
        runningCPU = -1;
      } else {
        // For Round Robin: if quantum expires, preempt.
        if (algorithm == 2) {
          currentQuantum--;
          if (currentQuantum == 0) {
            procList[runningCPU].state = READY_CPU;
            cpuQueue.push_back(runningCPU);
            runningCPU = -1;
          }
        }
      }
    } else {
      cpuTimeline.push_back("_");
    }

    // (4) Resource scheduling for Resource 1 (R1).
    if (runningRes1 == -1 && !resQueue1.empty()) {
      int idx = resQueue1.front();
      resQueue1.pop_front();
      runningRes1 = idx;
      procList[idx].state = RUNNING_RES;

      // Set remainingTime to the burst length (for the resource burst).
      procList[idx].remainingTime =
          procList[idx].tasks[procList[idx].curTask].time;
    }

    if (runningRes1 != -1) {
      procList[runningRes1].remainingTime--;
      resTimeline1.push_back(to_string(procList[runningRes1].id));
      if (procList[runningRes1].remainingTime == 0) {
        procList[runningRes1].curTask++;
        if (procList[runningRes1].curTask <
                procList[runningRes1].tasks.size() &&
            procList[runningRes1].tasks[procList[runningRes1].curTask].isCPU) {
          procList[runningRes1].state = READY_CPU;
          procList[runningRes1].remainingTime =
              procList[runningRes1].tasks[procList[runningRes1].curTask].time;
          cpuQueue.push_back(runningRes1);
        } else {
          procList[runningRes1].state = FINISHED;
          procList[runningRes1].finishTime = time + 1;
          finishedCount++;
        }
        runningRes1 = -1;
      }
    } else {
      resTimeline1.push_back("_");
    }

    // (5) Resource scheduling for Resource 2 (R2).
    if (runningRes2 == -1 && !resQueue2.empty()) {
      int idx = resQueue2.front();
      resQueue2.pop_front();
      runningRes2 = idx;
      procList[idx].state = RUNNING_RES;
      procList[idx].remainingTime =
          procList[idx].tasks[procList[idx].curTask].time;
    }

    if (runningRes2 != -1) {
      procList[runningRes2].remainingTime--;
      resTimeline2.push_back(to_string(procList[runningRes2].id));
      if (procList[runningRes2].remainingTime == 0) {
        procList[runningRes2].curTask++;
        if (procList[runningRes2].curTask <
                procList[runningRes2].tasks.size() &&
            procList[runningRes2].tasks[procList[runningRes2].curTask].isCPU) {
          procList[runningRes2].state = READY_CPU;
          procList[runningRes2].remainingTime =
              procList[runningRes2].tasks[procList[runningRes2].curTask].time;
          cpuQueue.push_back(runningRes2);
        } else {
          procList[runningRes2].state = FINISHED;
          procList[runningRes2].finishTime = time + 1;
          finishedCount++;
        }
        runningRes2 = -1;
      }
    } else {
      resTimeline2.push_back("_");
    }

    // Increment time for the next step.
    time++;

    // Increment waiting time for processes in the CPU queue.
    for (int idx : cpuQueue) {
      procList[idx].waitingTime++;
    }
  }
  for (int i = cpuTimeline.size() - 1; i >= 0; i--) {
    if (cpuTimeline[i] != "_") {
      lastCpuBusy = i;
      break;
    }
  }
}

void Scheduler::writeOutput(string outputFileName) {
  ofstream fout(outputFileName);
  if (!fout) {
    cerr << "Cannot open output file." << endl;
    exit(1);
  }

  // Output CPU Gantt chart.
  for (int i = 0; i <= lastCpuBusy; i++) {
    fout << cpuTimeline[i] << (i < lastCpuBusy ? " " : "");
  }
  fout << "\n";

  // Output resource timelines (for R1 and R2) in full.
  for (size_t i = 0; i < resTimeline1.size(); i++) {
    fout << resTimeline1[i] << (i < resTimeline1.size() - 1 ? " " : "");
  }
  fout << "\n";
  for (size_t i = 0; i < resTimeline2.size(); i++) {
    fout << resTimeline2[i] << (i < resTimeline2.size() - 1 ? " " : "");
  }
  fout << "\n";

  // Output turnaround times for each process (finish time minus arrival).
  for (int i = 0; i < numProc; i++) {
    int tat = procList[i].finishTime - procList[i].arrival;
    fout << tat << (i < numProc - 1 ? " " : "");
  }
  fout << "\n";

  // Output waiting times for each process.
  for (int i = 0; i < numProc; i++) {
    fout << procList[i].waitingTime << (i < numProc - 1 ? " " : "");
  }

  fout.close();
}