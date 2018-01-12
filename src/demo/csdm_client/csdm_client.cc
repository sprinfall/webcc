#include "csdm_client.h"

#include <iostream>
#include "boost/lexical_cast.hpp"
#include "csoap/csoap.h"

CsdmClient::CsdmClient() {
  Init();
}

bool CsdmClient::GetVersion(std::string* result_xml) {
  return Call0("getVersion", result_xml);
}

void CsdmClient::Init() {
  url_ = "/";

  service_ns_ = { "ser", "http://service.csdm.carestream.com/" };

  host_ = "127.0.0.1";
  port_ = "52540";

  result_name_ = "return";
}

bool CsdmClient::Call0(const std::string& operation,
                       std::string* result_xml) {
  return CallEx(operation, NULL, 0, result_xml);
}

bool CsdmClient::Call1(const std::string& operation,
                       const std::string& name,
                       const std::string& value,
                       std::string* result_xml) {
  csoap::Parameter parameters[] = {
    { name, value },
  };

  return CallEx(operation, parameters, 1, result_xml);
}

bool CsdmClient::Call2(const std::string& operation,
                       const std::string& name1,
                       const std::string& value1,
                       const std::string& name2,
                       const std::string& value2,
                       std::string* result_xml) {
  csoap::Parameter parameters[] = {
    { name1, value1 },
    { name2, value2 },
  };

  return CallEx(operation, parameters, 2, result_xml);
}

bool CsdmClient::CallEx(const std::string& operation,
                        const csoap::Parameter* parameters,
                        std::size_t count,
                        std::string* result) {
  csoap::Error error = Call(operation, parameters, count, result);
  return error == csoap::kNoError;
}
