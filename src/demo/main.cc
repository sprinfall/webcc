#include <iostream>
#include "demo/calculator.h"

int main() {
  demo::Calculator calculator;

  float x = 1.0;
  float y = 2.0;
  float result = 0.0;

  if (calculator.Add(x, y, &result)) {
    printf("add: %.1f\n", result);
  }

  if (calculator.Subtract(x, y, &result)) {
    printf("subtract: %.1f\n", result);
  }

  if (calculator.Multiply(x, y, &result)) {
    printf("multiply: %.1f\n", result);
  }

  if (calculator.Divide(x, y, &result)) {
    printf("divide: %.1f\n", result);
  }

  return 0;
}

// Output:
// add: 3.0
// subtract: -1.0
// multiply: 2.0
// divide: 0.5
