#ifndef CALCULATOR_CLIENT_H_
#define CALCULATOR_CLIENT_H_

// Wrapper of calculator.wsdl.
// See http://ws1.parasoft.com/glue/calculator.wsdl

#include <string>
#include "csoap/soap_client.h"

class CalculatorClient : csoap::SoapClient {
public:
  CalculatorClient();

  bool Add(double x, double y, double* result);

  bool Subtract(double x, double y, double* result);

  bool Multiply(double x, double y, double* result);

  bool Divide(double x, double y, double* result);

protected:
  void Init();

  // A more concrete wrapper to make a call.
  bool Calc(const std::string& operation,
            const std::string& x_name,
            const std::string& y_name,
            double x,
            double y,
            double* result);
};

#endif  // CALCULATOR_CLIENT_H_
