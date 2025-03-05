#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <queue>

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " <inputFile> <outputFile>\n";
    return EXIT_FAILURE;
  }

  string inputFile = argv[1];
  string outputFile = argv[2];

  scheduler *s = new scheduler(argv[1]);
  s->run();
  s->exportOutput(argv[2]);

  delete s;

  cout << "\n CPU scheduling successful, output written to " << outputFile
       << endl;

  return EXIT_SUCCESS;
}