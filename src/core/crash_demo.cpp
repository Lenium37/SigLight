//
// Created by Jan on 03.08.2019.
//
#include <iostream>
#include <vector>
#include <stacktrace_registration.h>

int main() {
  register_signal_handlers();
  std::cout << signal_handler_registration_report() << std::endl;

  std::vector<int> *test = nullptr;
  test->push_back(0);
}
