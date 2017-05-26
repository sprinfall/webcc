#include <iostream>
#include "demo/calculator.h"

int main() {
  demo::Calculator calculator;

  float result = 0.0;
  if (!calculator.Add(1.0, 2.0, &result)) {
    std::cerr << "Failed to call web service." << std::endl;
  } else {
    std::cout << "Add result: " << std::showpoint << result << std::endl;
  }

  return 0;
}
