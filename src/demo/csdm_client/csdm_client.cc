#include "csdm_client.h"

#include <iostream>
#include "boost/lexical_cast.hpp"
#include "csoap/csoap.h"

CsdmClient::CsdmClient() {
  Init();
}

bool CsdmClient::GetVersion(std::string* result_xml) {
  return Call("getVersion", NULL, 0, result_xml);
}

void CsdmClient::Init() {
  url_ = "/";

  service_ns_ = { "ser", "http://service.csdm.carestream.com/" };

  host_ = "127.0.0.1";
  port_ = "52540";

  result_name_ = "Result";
}

bool CsdmClient::Call1(const std::string& operation,
                       const std::string& name,
                       const std::string& value,
                       std::string* result_xml) {
  csoap::Parameter parameters[] = {
    { name, value },
  };

  return Call(operation, parameters, 1, result_xml);
}
