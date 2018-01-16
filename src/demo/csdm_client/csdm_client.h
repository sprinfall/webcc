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

  // Call CSDM API with NO parameter.
  bool Call0(const std::string& operation,
             std::string* result_xml);

  // Call CSDM API with one parameter.
  bool Call1(const std::string& operation,
             const std::string& name,
             const std::string& value,
             std::string* result_xml);

  // Call CSDM API with two parameters.
  bool Call2(const std::string& operation,
             const std::string& name1,
             const std::string& value1,
             const std::string& name2,
             const std::string& value2,
             std::string* result_xml);

  // A wrapper of CSoapClient::Call().
  bool CallEx(const std::string& operation,
              std::vector<csoap::Parameter>&& parameters,
              std::string* result);
};

#endif  // CSDM_CLIENT_H_
