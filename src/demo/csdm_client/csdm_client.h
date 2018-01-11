#ifndef CSDM_CLIENT_H_
#define CSDM_CLIENT_H_

// CSDM (CS Data Manager) is a project of my business project.

#include <string>
#include "csoap/soap_client.h"

class CsdmClient : csoap::SoapClient {
public:
  CsdmClient();

  bool GetVersion(std::string* result_xml);

protected:
  void Init();

  // Call CSDM API with one parameter.
  bool Call1(const std::string& operation,
             const std::string& name,
             const std::string& value,
             std::string* result_xml);
};

#endif  // CSDM_CLIENT_H_
