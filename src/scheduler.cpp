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
  // Vòng lặp chính của hệ thống
  while (true) {
    handlePendingResources();       // Xử lý các R đang chờ
    if (checkTermination()) break;  // Kiểm tra điều kiện kết thúc
    checkArrivals(time);            // Kiểm tra các tiến trình mới
    if (algorithm == 1)
      scheduleFCFS();
    else if (algorithm == 2)
      scheduleRR();
    else if (algorithm == 3)
      scheduleSJF();
    else if (algorithm == 4)
      scheduleSRTN();

    int currentRunningProcess =
        runningCPU;  // Lưu tiến trình đang chạy trước khi xử lý burst

    processCPUBurst();  // Xử lý CPU đang chạy
    updateWaitingTime(
        currentRunningProcess);  // Cập nhật thời gian chờ (nếu có)

    scheduleResource(1);  // Xử lý R1
    scheduleResource(2);  // Xử lý R2
    time++;               // Tăng thời gian hệ thống
  }
  determineLastCpuBusyTime();  // Xác định thời gian cuối cùng CPU chạy
}

void Scheduler::handlePendingResources() {
  // Xử lý các tiến trình chờ resource
  if (!pendingResource.empty()) {
    // Nếu có tiến trình chờ resource
    for (int idx : pendingResource) {
      // Duyệt qua các tiến trình chờ resource
      int resId = procList[idx].tasks[procList[idx].curTask].resourceId;
      procList[idx].state = READY_RES;
      (resId == 1 ? resQueue1 : resQueue2).push_back(idx);
    }
    pendingResource.clear();
  }
}

bool Scheduler::checkTermination() {
  // Kiểm tra điều kiện kết thúc
  // Nếu tất cả tiến trình đã chạy xong, CPU rãnh, không có tiến trình nào trong
  // queue và không có resource nào đang chạy

  return finishedCount == numProc && runningCPU == -1 && cpuQueue.empty() &&
         runningRes1 == -1 && runningRes2 == -1 && resQueue1.empty() &&
         resQueue2.empty();
}

void Scheduler::checkArrivals(int currentTime) {
  for (int i = 0; i < numProc; i++) {
    if (procList[i].state == NOT_ARRIVED &&
        procList[i].arrival == currentTime) {
      procList[i].state = READY_CPU;
      procList[i].readyCpuTime = currentTime;

      cpuQueue.push_back(i);
    }
  }

  // Xử lý các tiến trình đã chạy xong chờ vào hàng đợi

  for (int idx : readyToRun) {
    cpuQueue.push_back(idx);
  }
  readyToRun.clear();
}

void Scheduler::scheduleFCFS() {
  if (runningCPU == -1 && !cpuQueue.empty()) {
    // Nếu CPU đang rãnh và có tiến trình trong queue
    runningCPU = cpuQueue.front();
    cpuQueue.pop_front();

    procList[runningCPU].state = RUNNING_CPU;
  }
}

void Scheduler::scheduleRR() {
  if (runningCPU == -1 && !cpuQueue.empty()) {
    // Nếu CPU đang rãnh và có tiến trình trong queue
    runningCPU = cpuQueue.front();
    cpuQueue.pop_front();
    procList[runningCPU].state = RUNNING_CPU;
    currentQuantum = quantum;
  }
}

void Scheduler::scheduleSJF() {
  if (runningCPU == -1 && !cpuQueue.empty()) {
    // Nếu CPU đang rãnh và có tiến trình trong queue
    auto it = min_element(cpuQueue.begin(), cpuQueue.end(), [&](int a, int b) {
      if (procList[a].remainingTime != procList[b].remainingTime) {
        return procList[a].remainingTime < procList[b].remainingTime;
      }
      // Nếu remaining time bằng nhau, ưu tiên tiến trình đến sau
      return procList[a].arrival > procList[b].arrival;
    });
    runningCPU = *it;
    cpuQueue.erase(it);
    procList[runningCPU].state = RUNNING_CPU;
  }
}

void Scheduler::scheduleSRTN() {
  if (runningCPU == -1 && !cpuQueue.empty()) {
    // Nếu CPU đang rãnh và có tiến trình trong queue
    auto it = min_element(cpuQueue.begin(), cpuQueue.end(), [&](int a, int b) {
      if (procList[a].remainingTime != procList[b].remainingTime) {
        return procList[a].remainingTime < procList[b].remainingTime;
      }
      // Nếu remaining time bằng nhau, ưu tiên tiến trình đến sau
      return procList[a].arrival > procList[b].arrival;
    });
    runningCPU = *it;
    cpuQueue.erase(it);
    procList[runningCPU].state = RUNNING_CPU;

  } else if (runningCPU != -1 && !cpuQueue.empty()) {
    // Nếu CPU đang chạy và có tiến trình trong queue
    auto it = min_element(cpuQueue.begin(), cpuQueue.end(), [&](int a, int b) {
      // So sánh thời gian còn lại của CPU để chọn tiến trình chạy ngắn
      // nhất
      if (procList[a].remainingTime != procList[b].remainingTime) {
        return procList[a].remainingTime < procList[b].remainingTime;
      }
      // Nếu remaining time bằng nhau, ưu tiên tiến trình đến sau
      return procList[a].arrival > procList[b].arrival;
    });
    int candidate = *it;  // Tiến trình có thời gian còn lại ngắn nhất

    if (procList[candidate].remainingTime <
            procList[runningCPU].remainingTime ||
        (procList[candidate].remainingTime ==
             procList[runningCPU].remainingTime &&
         procList[candidate].arrival > procList[runningCPU].arrival)) {
      // Preempt nếu tiến trình mới có remaining time ngắn hơn
      // hoặc bằng nhau nhưng đến sau
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
  } else if (algorithm == 2) {
    currentQuantum--;

    if (currentQuantum == 0) {
      // Lưu process hiện tại
      int currentProcess = runningCPU;

      // Reset CPU và trạng thái của process hiện tại
      runningCPU = -1;
      procList[currentProcess].state = READY_CPU;

      // Thêm process hiện tại vào cuối queue ngay lập tức
      // (thời điểm time hiện tại)
      readyToRun.push_back(currentProcess);
      // Reset quantum
    }
  }
}

// Xử lý khi một CPU đã chạy xong
void Scheduler::completeCPUExecution() {
  procList[runningCPU].curTask++;  // Chuyển sang tác vụ tiếp theo
  if (procList[runningCPU].curTask < procList[runningCPU].tasks.size() &&
      !procList[runningCPU].tasks[procList[runningCPU].curTask].isCPU) {
    // Nếu tiến trình cần chạy resource
    pendingResource.push_back(runningCPU);
  } else {
    // Nếu tiến trình đã chạy hết tác vụ
    procList[runningCPU].state = FINISHED;
    procList[runningCPU].finishTime = time + 1;
    finishedCount++;
  }

  // Đặt lại CPU về trạng thái rãnh
  runningCPU = -1;
}

// Xử lý khi một resource cần được chạy
void Scheduler::scheduleResource(int resId) {
  int &runningRes =
      (resId == 1) ? runningRes1 : runningRes2;  // Lưu resource đang chạy
  deque<int> &resQueue =
      (resId == 1) ? resQueue1 : resQueue2;  // Lấy queue của resource
  vector<string> &resTimeline =
      (resId == 1) ? resTimeline1 : resTimeline2;  // Lấy timeline của resource

  if (runningRes == -1 && !resQueue.empty()) {
    // Nếu resource đang rãnh và có resource trong queue
    runningRes = resQueue.front();
    resQueue.pop_front();
    procList[runningRes].state = RUNNING_RES;
    procList[runningRes].remainingTime =
        procList[runningRes].tasks[procList[runningRes].curTask].time;
  }

  if (runningRes != -1) {
    // Nếu resource đang chạy
    procList[runningRes].remainingTime--;
    resTimeline.push_back(to_string(procList[runningRes].id));
    if (procList[runningRes].remainingTime == 0)
      completeResourceExecution(runningRes);
  } else {
    // Nếu resource không chạy
    resTimeline.push_back("_");
  }
}

// Xử lý khi một resource đã chạy xong (hoặc bị gián đoạn)
void Scheduler::completeResourceExecution(int &runningRes) {
  procList[runningRes].curTask++;  // Chuyển sang tác vụ tiếp theo

  if (procList[runningRes].curTask < procList[runningRes].tasks.size() &&
      procList[runningRes].tasks[procList[runningRes].curTask].isCPU) {
    // Nếu resource chưa chạy hết tác vụ CPU
    procList[runningRes].state = READY_CPU;
    procList[runningRes].readyCpuTime = time;
    procList[runningRes].remainingTime =
        procList[runningRes].tasks[procList[runningRes].curTask].time;
    readyToRun.push_back(runningRes);
  } else {
    // Nếu resource đã chạy hết tác vụ
    procList[runningRes].state = FINISHED;
    procList[runningRes].finishTime = time + 1;
    finishedCount++;
  }

  runningRes = -1;  // Đặt lại resource đang chạy về trạng thái idle
}

void Scheduler::updateWaitingTime(int curRunningProcess) {
  for (int idx : cpuQueue) {
    // Bỏ qua tiến trình đang chạy
    if (idx == curRunningProcess) {
      continue;
    }

    // Kiểm tra các điều kiện:
    // 1. Tiến trình đã đến (thời gian hiện tại >= thời gian đến)
    // 2. Tiến trình đang ở trạng thái READY_CPU
    // 3. Tiến trình không đang chạy
    // 4. Đối với tiến trình từ resource: thời gian hiện tại > thời điểm sẵn
    // sàng CPU
    // 5. Đối với tiến trình mới đến: thời gian hiện tại >= thời gian đến
    if (procList[idx].state == READY_CPU && idx != runningCPU) {
      // Xử lý 2 trường hợp: tiến trình mới đến và tiến trình từ resource
      if (procList[idx].curTask == 0) {
        // Trường hợp 1: Tiến trình mới, chưa chạy lần nào
        // Tính thời gian chờ khi tiến trình đã đến
        if (time >= procList[idx].arrival) {
          procList[idx].waitingTime++;
        }
      } else {
        // Trường hợp 2: Tiến trình đã chạy xong resource
        // Chỉ tính thời gian chờ sau khi đã sẵn sàng cho CPU
        if (time > procList[idx].readyCpuTime) {
          procList[idx].waitingTime++;
        }
      }
    }
  }
}

// Xác định thời gian cuối cùng CPU chạy
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