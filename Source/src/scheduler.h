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
  int algorithm;             // Thuật toán lập lịch
  int quantum;               // Quantum của Round Robin
  int numProc;               // Số tiến trình
  vector<Process> procList;  // Danh sách các tiến trình

  int time;           // Thời gian hiện tại
  int finishedCount;  // Số tiến trình đã hoàn thành

  int runningCPU;  // index của tiến trình đang chạy CPU (>=0) hoặc rãnh (-1)
  int currentQuantum;  // for Round Robin

  deque<int> cpuQueue;   // CPU waiting queue
  deque<int> resQueue1;  // resource R1 waiting queue
  deque<int> resQueue2;  // resource R2 waiting queue

  // Timeline của CPU và các resource
  vector<string> cpuTimeline;
  vector<string> resTimeline1;
  vector<string> resTimeline2;

  // Trạng thái của resource
  // -1: rãnh, >=0: index của tiến trình đang chạy resource
  int runningRes1;
  int runningRes2;

  // Vector các tiến trình đang chờ resource
  vector<int> pendingResource;
  int lastCpuBusy;

  // Đánh dấu các tiến trình đã chạy xong task chuẩn bị quay lại cpu
  vector<int> readyToRun;

 public:
  Scheduler();

  void readInput(string inputFileName);
  void schedule();
  void handlePendingResources();
  bool checkTermination();
  void checkArrivals(int currentTime);
  void scheduleFCFS();
  void scheduleRR();
  void scheduleSJF();
  void scheduleSRTN();
  void processCPUBurst();
  void completeCPUExecution();
  void scheduleResource(int resId);
  void completeResourceExecution(int &runningRes);
  void updateWaitingTime(int curRunningProcess);
  void determineLastCpuBusyTime();
  void writeOutput(string outputFileName);
};
