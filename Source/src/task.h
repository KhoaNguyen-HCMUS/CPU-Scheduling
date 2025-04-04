#pragma once

struct Task {
  bool isCPU;      // true for CPU burst, false for resource burst
  int time;        // burst duration
  int resourceId;  // valid only if isCPU==false; e.g. 1 for R1, 2 for R2

  Task() : isCPU(true), time(0), resourceId(0) {}
  Task(bool cpu, int t, int rid = 0) : isCPU(cpu), time(t), resourceId(rid) {}
};