#include <iostream>

#include "webcc/logger.h"
#include "webcc/soap_client.h"

// -----------------------------------------------------------------------------

class CalcClient {
 public:
  CalcClient(const std::string& host, const std::string& port)
      : soap_client_(host, port) {
    soap_client_.SetTimeout(5);

    soap_client_.set_url("/glue/calculator");
    soap_client_.set_service_ns({
      "cal", "http://www.parasoft.com/wsdl/calculator/"
    });
    soap_client_.set_result_name("Result");

    // Customize request XML format.
    soap_client_.set_format_raw(false);
    soap_client_.set_indent_str("  ");
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

 private:
  bool Calc(const std::string& operation,
            const std::string& x_name, const std::string& y_name,
            double x, double y,
            double* result) {
    std::vector<webcc::SoapParameter> parameters{
      { x_name, x },
      { y_name, y }
    };

    std::string result_str;
    if (!soap_client_.Request(operation, std::move(parameters), &result_str)) {
      PrintError();
      return false;
    }

    try {
      *result = std::stod(result_str);
    } catch (const std::exception&) {
      return false;
    }

    return true;
  }

  void PrintError() {
    std::cout << webcc::DescribeError(soap_client_.error());
    if (soap_client_.timed_out()) {
      std::cout << " (timed out)";
    }
    std::cout << std::endl;
  }

  webcc::SoapClient soap_client_;
};

// -----------------------------------------------------------------------------

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  CalcClient calc("ws1.parasoft.com", "");  // Use default port 80

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
