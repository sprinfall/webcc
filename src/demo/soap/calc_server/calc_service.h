#ifndef CALC_SERVICE_H_
#define CALC_SERVICE_H_

#include "csoap/soap_service.h"

class CalcService : public csoap::SoapService {
public:
  CalcService() = default;
  ~CalcService() override = default;

  bool Handle(const csoap::SoapRequest& soap_request,
              csoap::SoapResponse* soap_response) override;
};

#endif  // CALC_SERVICE_H_
