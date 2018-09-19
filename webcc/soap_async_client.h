#ifndef WEBCC_SOAP_ASYNC_CLIENT_H_
#define WEBCC_SOAP_ASYNC_CLIENT_H_

#include <functional>
#include <string>
#include <vector>

#include "webcc/http_async_client.h"
#include "webcc/soap_message.h"
#include "webcc/soap_parameter.h"

namespace webcc {

// Response handler/callback.
typedef std::function<void(std::string, Error, bool)> SoapResponseHandler;

class SoapAsyncClient {
 public:
  // If |port| is empty, |host| will be checked to see if it contains port or
  // not (separated by ':').
  SoapAsyncClient(boost::asio::io_context& io_context,  // NOLINT
                  const std::string& host, const std::string& port = "",
                  SoapVersion soap_version = kSoapV12);

  ~SoapAsyncClient() = default;

  WEBCC_DELETE_COPY_ASSIGN(SoapAsyncClient);

  void set_timeout_seconds(int timeout_seconds) {
    timeout_seconds_ = timeout_seconds;
  }

  void set_url(const std::string& url) { url_ = url; }

  void set_service_ns(const SoapNamespace& service_ns) {
    service_ns_ = service_ns;
  }

  void set_result_name(const std::string& result_name) {
    result_name_ = result_name;
  }

  void set_format_raw(bool format_raw) { format_raw_ = format_raw; }

  void set_indent_str(const std::string& indent_str) {
    indent_str_ = indent_str;
  }

  void Request(const std::string& operation,
               std::vector<SoapParameter>&& parameters,
               SoapResponseHandler soap_response_handler);

 private:
  void ResponseHandler(SoapResponseHandler soap_response_handler,
                       HttpResponsePtr http_response,
                       Error error, bool timed_out);

  boost::asio::io_context& io_context_;

  std::string host_;
  std::string port_;  // Leave this empty to use default 80.

  SoapVersion soap_version_;

  // Request URL.
  std::string url_;

  // Namespace for your web service.
  SoapNamespace service_ns_;

  // Response result XML node name.
  // E.g., "Result".
  std::string result_name_;

  // Format request XML without any indentation or line breaks.
  bool format_raw_;

  // Indent string for request XML.
  // Applicable when |format_raw_| is false.
  std::string indent_str_;

  // Timeout in seconds; only effective when > 0.
  int timeout_seconds_;
};

}  // namespace webcc

#endif  // WEBCC_SOAP_ASYNC_CLIENT_H_
