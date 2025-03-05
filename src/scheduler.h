#pragma once
#include <string>
using namespace std;

class scheduler {
	private:
	
 public:
  scheduler(string inputFile);
  void run();
  void exportOutput(string outputFile);
};