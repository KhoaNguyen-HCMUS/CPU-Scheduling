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
  //Vòng lặp chính của hệ thống
  while (true) {
    handlePendingResources(); //Xử lý các R đang chờ
    if (checkTermination()) break; //Kiểm tra điều kiện kết thúc
    checkArrivals();  //Kiểm tra các tiến trình mới
    if (algorithm == 1)
      scheduleFCFS();
    else if (algorithm == 2)
      scheduleRR();
    else if (algorithm == 3)
      scheduleSJF();
    else if (algorithm == 4)
      scheduleSRTN();
    processCPUBurst();  //Xử lý CPU đang chạy
    scheduleResource(1); //Xử lý R1
    scheduleResource(2);  //Xử lý R2
    updateWaitingTime();  //Cập nhật thời gian chờ (nếu có)
    time++; //Tăng thời gian hệ thống
  }
  determineLastCpuBusyTime(); //Xác định thời gian cuối cùng CPU chạy
}

void Scheduler::handlePendingResources() {
  if (!pendingResource.empty()) {
    for (int idx : pendingResource) {
      int resId = procList[idx].tasks[procList[idx].curTask].resourceId;
      procList[idx].state = READY_RES;
      (resId == 1 ? resQueue1 : resQueue2).push_back(idx);
    }
    pendingResource.clear();
  }
}

bool Scheduler::checkTermination() {
  return finishedCount == numProc && runningCPU == -1 && cpuQueue.empty() &&
         runningRes1 == -1 && runningRes2 == -1 && resQueue1.empty() &&
         resQueue2.empty();
}

void Scheduler::checkArrivals() {
  for (int i = 0; i < numProc; i++) {
    if (procList[i].state == NOT_ARRIVED && procList[i].arrival == time) {
      procList[i].state = READY_CPU;
      cpuQueue.push_back(i);
    }
  }
}

void Scheduler::scheduleFCFS() {
  if (runningCPU == -1 && !cpuQueue.empty()) {
    runningCPU = cpuQueue.front();
    cpuQueue.pop_front();
    procList[runningCPU].state = RUNNING_CPU;
  }
}

void Scheduler::scheduleRR() {
  if (runningCPU == -1 && !cpuQueue.empty()) {
    runningCPU = cpuQueue.front();
    cpuQueue.pop_front();
    procList[runningCPU].state = RUNNING_CPU;
    currentQuantum = quantum;
  }
}

void Scheduler::scheduleSJF() {
  if (runningCPU == -1 && !cpuQueue.empty()) {
    auto it = min_element(cpuQueue.begin(), cpuQueue.end(), [&](int a, int b) {
      return procList[a].remainingTime < procList[b].remainingTime;
    });
    runningCPU = *it;
    cpuQueue.erase(it);
    procList[runningCPU].state = RUNNING_CPU;
  }
}

void Scheduler::scheduleSRTN() {
  if (runningCPU == -1 && !cpuQueue.empty()) {
    auto it = min_element(cpuQueue.begin(), cpuQueue.end(), [&](int a, int b) {
      return procList[a].remainingTime < procList[b].remainingTime;
    });
    runningCPU = *it;
    cpuQueue.erase(it);
    procList[runningCPU].state = RUNNING_CPU;
  } else if (runningCPU != -1 && !cpuQueue.empty()) {
    auto it = min_element(cpuQueue.begin(), cpuQueue.end(), [&](int a, int b) {
      return procList[a].remainingTime < procList[b].remainingTime;
    });
    int candidate = *it;
    if (procList[candidate].remainingTime <
        procList[runningCPU].remainingTime) {
      procList[runningCPU].state = READY_CPU;
      cpuQueue.push_back(runningCPU);
      runningCPU = candidate;
      cpuQueue.erase(it);
      procList[runningCPU].state = RUNNING_CPU;
    }
  }
}

void Scheduler::processCPUBurst() {
  if (runningCPU == -1) {
    cpuTimeline.push_back("_");
    return;
  }

  procList[runningCPU].remainingTime--;
  cpuTimeline.push_back(to_string(procList[runningCPU].id));

  if (procList[runningCPU].remainingTime == 0) {
    completeCPUExecution();
  } else if (algorithm == 2 && --currentQuantum == 0) {
    procList[runningCPU].state = READY_CPU;
    cpuQueue.push_back(runningCPU);
    runningCPU = -1;
  }
}

void Scheduler::completeCPUExecution() {
  procList[runningCPU].curTask++;
  if (procList[runningCPU].curTask < procList[runningCPU].tasks.size() &&
      !procList[runningCPU].tasks[procList[runningCPU].curTask].isCPU) {
    pendingResource.push_back(runningCPU);
  } else {
    procList[runningCPU].state = FINISHED;
    procList[runningCPU].finishTime = time + 1;
    finishedCount++;
  }
  runningCPU = -1;
}

void Scheduler::scheduleResource(int resId) {
  int &runningRes = (resId == 1) ? runningRes1 : runningRes2;
  deque<int> &resQueue = (resId == 1) ? resQueue1 : resQueue2;
  vector<string> &resTimeline = (resId == 1) ? resTimeline1 : resTimeline2;

  if (runningRes == -1 && !resQueue.empty()) {
    runningRes = resQueue.front();
    resQueue.pop_front();
    procList[runningRes].state = RUNNING_RES;
    procList[runningRes].remainingTime =
        procList[runningRes].tasks[procList[runningRes].curTask].time;
  }

  if (runningRes != -1) {
    procList[runningRes].remainingTime--;
    resTimeline.push_back(to_string(procList[runningRes].id));
    if (procList[runningRes].remainingTime == 0)
      completeResourceExecution(runningRes);
  } else {
    resTimeline.push_back("_");
  }
}

void Scheduler::completeResourceExecution(int &runningRes) {
  procList[runningRes].curTask++;
  if (procList[runningRes].curTask < procList[runningRes].tasks.size() &&
      procList[runningRes].tasks[procList[runningRes].curTask].isCPU) {
    procList[runningRes].state = READY_CPU;
    procList[runningRes].remainingTime =
        procList[runningRes].tasks[procList[runningRes].curTask].time;
    cpuQueue.push_back(runningRes);
  } else {
    procList[runningRes].state = FINISHED;
    procList[runningRes].finishTime = time + 1;
    finishedCount++;
  }
  runningRes = -1;
}

void Scheduler::updateWaitingTime() {
  for (int idx : cpuQueue) procList[idx].waitingTime++;
}

void Scheduler::determineLastCpuBusyTime() {
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