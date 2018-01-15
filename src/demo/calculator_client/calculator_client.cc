#include "calculator_client.h"

#include <iostream>
#include "boost/lexical_cast.hpp"

CalculatorClient::CalculatorClient() {
  Init();
}

bool CalculatorClient::Add(double x, double y, double* result) {
  return Calc("add", "x", "y", x, y, result);
}

bool CalculatorClient::Subtract(double x, double y, double* result) {
  return Calc("subtract", "x", "y", x, y, result);
}

bool CalculatorClient::Multiply(double x, double y, double* result) {
  return Calc("multiply", "x", "y", x, y, result);
}

bool CalculatorClient::Divide(double x, double y, double* result) {
  return Calc("divide", "numerator", "denominator", x, y, result);
}

// Set to 0 to test our own calculator server created with csoap.
#define ACCESS_PARASOFT 1

void CalculatorClient::Init() {
#if ACCESS_PARASOFT
  url_ = "/glue/calculator";
  host_ = "ws1.parasoft.com";
  port_ = "80";  // Or leave it empty because 80 is the default HTTP port.
  service_ns_ = { "ser", "http://www.parasoft.com/wsdl/calculator/" };
  result_name_ = "Result";
#else
  url_ = "/";
  host_ = "localhost";
  port_ = "8080";
  service_ns_ = { "ser", "http://mycalculator/" };
  result_name_ = "Result";
#endif
}

bool CalculatorClient::Calc(const std::string& operation,
                            const std::string& x_name,
                            const std::string& y_name,
                            double x,
                            double y,
                            double* result) {
  csoap::Parameter parameters[] = {
    { x_name, x },
    { y_name, y }
  };

  std::string result_str;
  csoap::Error error = Call(operation, parameters, 2, &result_str);

  if (error != csoap::kNoError) {
    std::cerr << "Error: " << error;
    std::cerr << ", " << csoap::GetErrorMessage(error) << std::endl;
    return false;
  }

  try {
    *result = boost::lexical_cast<double>(result_str);
  } catch (boost::bad_lexical_cast&) {
    return false;
  }

  return true;
}
