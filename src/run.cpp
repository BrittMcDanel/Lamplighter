#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
using namespace std;

// remove later current host ip: 174.59.31.92:6112
int main(int argc, char *argv[]) {
  int make_status = system("ninja main");
  if (make_status == 0) {
    if (argc != 2) {
      cout << "Expected --server or --client after ./run. Example: ./run "
              "--server\n";
      abort();
    }
    string server_or_client_str = string(argv[1]);
    string run_main_str = "./main 127.0.0.01:6112 " + server_or_client_str;
    int main_status = system(run_main_str.c_str());
  }
  return 0;
}