#ifndef CALC_CLIENT_H_
#define CALC_CLIENT_H_

#include <string>

#include "webcc/soap_client.h"

class CalcClient : public webcc::SoapClient {
public:
  CalcClient();

  bool Add(double x, double y, double* result);

  bool Subtract(double x, double y, double* result);

  bool Multiply(double x, double y, double* result);

  bool Divide(double x, double y, double* result);

  // For testing purpose.
  bool NotExist(double x, double y, double* result);

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

#endif  // CALC_CLIENT_H_
