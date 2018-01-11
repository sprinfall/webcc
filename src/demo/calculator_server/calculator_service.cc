#include "calculator_service.h"

#include "boost/lexical_cast.hpp"

#include "csoap/soap_request.h"
#include "csoap/soap_response.h"

CalculatorService::CalculatorService() {
}

bool CalculatorService::Handle(const csoap::SoapRequest& request,
                               csoap::SoapResponse* response) {
  try {
    if (request.operation() == "add") {
      double x = boost::lexical_cast<double>(request.GetParameter("x"));
      double y = boost::lexical_cast<double>(request.GetParameter("y"));

      double result = x + y;

      response->set_soapenv_ns(csoap::kSoapEnvNamespace);
      response->set_service_ns({ "ser", "http://mycalculator/" });
      response->set_operation(request.operation());
      response->set_result_name("Result");
      response->set_result(boost::lexical_cast<std::string>(result));

      return true;

    } else {
      // NOT_IMPLEMENTED
    }
  } catch (boost::bad_lexical_cast&) {
    // BAD_REQUEST
  }

  return false;
}
