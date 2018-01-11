#ifndef CALCULATOR_SERVICE_H_
#define CALCULATOR_SERVICE_H_

#include "csoap/soap_service.h"

class CalculatorService : public csoap::SoapService {
public:
  CalculatorService();

  bool Handle(const csoap::SoapRequest& request,
              csoap::SoapResponse* response) override;

protected:
};

#endif  // CALCULATOR_SERVICE_H_
