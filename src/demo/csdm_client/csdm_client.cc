#include "csdm_client.h"
#include <iostream>

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
  std::vector<csoap::Parameter> parameters;
  return CallEx(operation, std::move(parameters), result_xml);
}

bool CsdmClient::Call1(const std::string& operation,
                       const std::string& name,
                       const std::string& value,
                       std::string* result_xml) {
  std::vector<csoap::Parameter> parameters{
    { name, value },
  };
  return CallEx(operation, std::move(parameters), result_xml);
}

bool CsdmClient::Call2(const std::string& operation,
                       const std::string& name1,
                       const std::string& value1,
                       const std::string& name2,
                       const std::string& value2,
                       std::string* result_xml) {
  std::vector<csoap::Parameter> parameters{
    { name1, value1 },
    { name2, value2 },
  };
  return CallEx(operation, std::move(parameters), result_xml);
}

bool CsdmClient::CallEx(const std::string& operation,
                        std::vector<csoap::Parameter>&& parameters,
                        std::string* result) {
  csoap::Error error = Call(operation, std::move(parameters), result);
  return error == csoap::kNoError;
}
