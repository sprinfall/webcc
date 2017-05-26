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

protected:
  void Init();

  bool Call(const std::string& operation,
            const csoap::Parameter* parameters,
            size_t count,
            std::string* result);

protected:
  std::string url_;  // Request URL

  std::string host_;
  std::string port_;

  csoap::Namespace soap_envelope_ns_;
  csoap::Namespace service_ns_;
};

}  // namespace demo

#endif  // DEMO_CALCULATOR_H_
