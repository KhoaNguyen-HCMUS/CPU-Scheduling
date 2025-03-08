#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <deque>
#include <string>
#include <algorithm>
#include <map>

#include "task.h"
#include "process.h"
#include "scheduler.h"

using namespace std;

int main(int argc, char* argv[]) {
  // Check command-line arguments.
  if (argc < 3) {
    cerr << "Usage: " << argv[0] << " <INPUT_FILE> <OUTPUT_FILE>" << endl;
    return 1;
  }

  string inputFileName = argv[1];
  string outputFileName = argv[2];

  Scheduler scheduler;
  scheduler.readInput(inputFileName);
  scheduler.schedule();
  scheduler.writeOutput(outputFileName);
}
