#include <iostream>
#include "webcc/logger.h"
#include "calc_client.h"

int main() {
  LOG_INIT(webcc::VERB, 0);

  CalcClient calc;

  double x = 1.0;
  double y = 2.0;
  double result = 0.0;

  if (calc.Add(x, y, &result)) {
    printf("add: %.1f\n", result);
  }

  if (calc.Subtract(x, y, &result)) {
    printf("subtract: %.1f\n", result);
  }

  if (calc.Multiply(x, y, &result)) {
    printf("multiply: %.1f\n", result);
  }

  if (calc.Divide(x, y, &result)) {
    printf("divide: %.1f\n", result);
  }

  // Try to call a non-existing operation.
  if (calc.NotExist(x, y, &result)) {
    printf("notexist: %.1f\n", result);
  }

  return 0;
}
