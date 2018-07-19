#include <iostream>

#include "webcc/logger.h"
#include "webcc/soap_client.h"

// -----------------------------------------------------------------------------

class CalcClient : public webcc::SoapClient {
 public:
  CalcClient(const std::string& host, const std::string& port)
      : webcc::SoapClient(host, port) {
    timeout_seconds_ = 5;  // Override the default timeout.

    url_ = "/glue/calculator";
    service_ns_ = { "cal", "http://www.parasoft.com/wsdl/calculator/" };
    result_name_ = "Result";

    // Customize request XML format.
    format_raw_ = false;
    indent_str_ = "  ";
  }

  bool Add(double x, double y, double* result) {
    return Calc("add", "x", "y", x, y, result);
  }

  bool Subtract(double x, double y, double* result) {
    return Calc("subtract", "x", "y", x, y, result);
  }

  bool Multiply(double x, double y, double* result) {
    return Calc("multiply", "x", "y", x, y, result);
  }

  bool Divide(double x, double y, double* result) {
    return Calc("divide", "numerator", "denominator", x, y, result);
  }

 protected:
  bool Calc(const std::string& operation,
            const std::string& x_name, const std::string& y_name,
            double x, double y,
            double* result) {
    std::vector<webcc::SoapParameter> parameters{
      { x_name, x },
      { y_name, y }
    };

    std::string result_str;
    webcc::Error error = Call(operation, std::move(parameters), &result_str);

    if (error != webcc::kNoError) {
      LOG_ERRO("Operation '%s' failed: %s", operation.c_str(),
               webcc::DescribeError(error));
      return false;
    }

    try {
      *result = std::stod(result_str);
    } catch (const std::exception&) {
      return false;
    }

    return true;
  }
};

// -----------------------------------------------------------------------------

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  // Default port 80.
  CalcClient calc("ws1.parasoft.com", "");

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

  return 0;
}
