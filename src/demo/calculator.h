#ifndef DEMO_CALCULATOR_H_
#define DEMO_CALCULATOR_H_

// Wrapper of calculator.wsdl.
// See http://service-repository.com/service/overview/877027757

#include <string>
#include "csoap/common.h"

namespace demo {

class Calculator {
public:
  Calculator();

  bool Add(float x, float y, float* result);

  bool Subtract(float x, float y, float* result);

  bool Multiply(float x, float y, float* result);

  bool Divide(float x, float y, float* result);

protected:
  void Init();

  // A more concrete wrapper to make a call.
  bool Calc(const std::string& operation,
            const std::string& x_name,
            const std::string& y_name,
            float x,
            float y,
            float* result);

  // A generic wrapper to make a call.
  bool Call(const std::string& operation,
            const csoap::Parameter* parameters,
            size_t count,
            std::string* result);

protected:
  // Request URL.
  // Could be a complete URL (http://ws1.parasoft.com/glue/calculator)
  // or just the path component of it (/glue/calculator).
  std::string url_;

  std::string host_;
  std::string port_;  // Leave this empty to use default 80.

  // The namespace of your service.
  csoap::Namespace service_ns_;
};

}  // namespace demo

#endif  // DEMO_CALCULATOR_H_
