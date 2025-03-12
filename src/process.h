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
  int id;              // id của tiến trình (bắt đầu từ 1)
  int arrival;         // thời điểm đến của tiến trình
  vector<Task> tasks;  // thứ tự các task (CPU hoặc resource)
  int curTask;         // index của task hiện tại
  int remainingTime;   // thời gian còn lại của burst hiện tại
  State state;         // trạng thái của tiến trình
  int finishTime;   // thời gian kết thúc
  int waitingTime;  // tổng thời gian chờ
  int readyCpuTime; //thời gian sẵn sàng cho CPU
  Process();

  Process& operator=(const Process& p);
};
