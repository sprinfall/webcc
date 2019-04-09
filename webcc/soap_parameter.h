#ifndef WEBCC_SOAP_PARAMETER_H_
#define WEBCC_SOAP_PARAMETER_H_

#include <string>

#include "webcc/globals.h"  // for COPY_ASSIGN_MOVE_DEFAULT

namespace webcc {

// Key-value SOAP parameter.
class SoapParameter {
public:
  SoapParameter() : as_cdata_(false) {
  }

  SoapParameter(const SoapParameter&) = default;
  SoapParameter& operator=(const SoapParameter&) = default;

  SoapParameter(const std::string& key, const char* value)
      : key_(key), value_(value),
        as_cdata_(false) {
  }

  SoapParameter(const std::string& key, const std::string& value,
                bool as_cdata = false)
      : key_(key), value_(value), as_cdata_(as_cdata) {
  }

  SoapParameter(const std::string& key, std::string&& value,
                bool as_cdata = false)
      : key_(key), value_(std::move(value)), as_cdata_(as_cdata) {
  }

  SoapParameter(const std::string& key, int value)
      : key_(key), value_(std::to_string(value)),
        as_cdata_(false) {
  }

  SoapParameter(const std::string& key, double value)
      : key_(key), value_(std::to_string(value)),
        as_cdata_(false) {
  }

  SoapParameter(const std::string& key, bool value)
      : key_(key), value_(value ? "true" : "false"),
        as_cdata_(false) {
  }

#if WEBCC_DEFAULT_MOVE_COPY_ASSIGN

  SoapParameter(SoapParameter&&) = default;
  SoapParameter& operator=(SoapParameter&&) = default;

#else

  SoapParameter(SoapParameter&& rhs)
      : key_(std::move(rhs.key_)),
        value_(std::move(rhs.value_)),
        as_cdata_(rhs.as_cdata_) {
  }

  SoapParameter& operator=(SoapParameter&& rhs) {
    if (&rhs != this) {
      key_ = std::move(rhs.key_);
      value_ = std::move(rhs.value_);
      as_cdata_ = rhs.as_cdata_;
    }
    return *this;
  }

#endif  // WEBCC_DEFAULT_MOVE_COPY_ASSIGN

  const std::string& key() const { return key_; }
  const std::string& value() const { return value_; }

  const char* c_key() const { return key_.c_str(); }
  const char* c_value() const { return value_.c_str(); }

  bool as_cdata() const { return as_cdata_; }

private:
  std::string key_;
  std::string value_;
  bool as_cdata_;
};

}  // namespace webcc

#endif  // WEBCC_SOAP_PARAMETER_H_
