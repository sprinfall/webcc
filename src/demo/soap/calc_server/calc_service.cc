#include "calc_service.h"

#include "boost/lexical_cast.hpp"

#include "csoap/soap_request.h"
#include "csoap/soap_response.h"

bool CalcService::Handle(const csoap::SoapRequest& soap_request,
                         csoap::SoapResponse* soap_response) {
  try {
    if (soap_request.operation() == "add") {
      double x = boost::lexical_cast<double>(soap_request.GetParameter("x"));
      double y = boost::lexical_cast<double>(soap_request.GetParameter("y"));

      double result = x + y;

      soap_response->set_soapenv_ns(csoap::kSoapEnvNamespace);
      soap_response->set_service_ns({
        "cal",
        "http://www.example.com/calculator/"
      });
      soap_response->set_operation(soap_request.operation());
      soap_response->set_result_name("Result");
      soap_response->set_result(std::to_string(result));

      return true;

    } else {
      // NOT_IMPLEMENTED
    }
  } catch (boost::bad_lexical_cast&) {
    // BAD_REQUEST
  }

  return false;
}
