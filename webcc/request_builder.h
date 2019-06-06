#ifndef WEBCC_REQUEST_BUILDER_H_
#define WEBCC_REQUEST_BUILDER_H_

#include <string>
#include <vector>

#include "webcc/request.h"

namespace webcc {

class RequestBuilder {
public:
  RequestBuilder() = default;
  ~RequestBuilder() = default;

  RequestBuilder(const RequestBuilder&) = delete;
  RequestBuilder& operator=(const RequestBuilder&) = delete;

  // Build the request.
  RequestPtr operator()();

  RequestBuilder& Get()     { return Method(methods::kGet);     }
  RequestBuilder& Head()    { return Method(methods::kHead);    }
  RequestBuilder& Post()    { return Method(methods::kPost);    }
  RequestBuilder& Put()     { return Method(methods::kPut);     }
  RequestBuilder& Delete()  { return Method(methods::kDelete);  }
  RequestBuilder& Patch()   { return Method(methods::kPatch);   }

  // NOTE:
  // The naming convention doesn't follow Google C++ Style for
  // consistency and simplicity.

  RequestBuilder& Method(const std::string& method) {
    method_ = method;
    return *this;
  }

  RequestBuilder& Url(const std::string& url) {
    url_ = url;
    return *this;
  }

  RequestBuilder& Query(const std::string& key, const std::string& value) {
    parameters_.push_back(key);
    parameters_.push_back(value);
    return *this;
  }

  RequestBuilder& Data(const std::string& data) {
    data_ = data;
    return *this;
  }

  RequestBuilder& Data(std::string&& data) {
    data_ = std::move(data);
    return *this;
  }

  RequestBuilder& Json(bool json = true) {
    json_ = json;
    return *this;
  }

  // Upload a file.
  RequestBuilder& File(const std::string& name, const Path& path,
                       const std::string& media_type = "");

  RequestBuilder& Form(FormPartPtr part) {
    form_parts_.push_back(part);
    return *this;
  }

  RequestBuilder& Form(const std::string& name, std::string&& data,
                       const std::string& media_type = "");

  RequestBuilder& Gzip(bool gzip = true) {
    gzip_ = gzip;
    return *this;
  }

  RequestBuilder& Header(const std::string& key, const std::string& value) {
    headers_.push_back(key);
    headers_.push_back(value);
    return *this;
  }

  RequestBuilder& KeepAlive(bool keep_alive = true) {
    keep_alive_ = keep_alive;
    return *this;
  }

  RequestBuilder& Auth(const std::string& type, const std::string& credentials);

  RequestBuilder& AuthBasic(const std::string& login,
                            const std::string& password);

  RequestBuilder& AuthToken(const std::string& token);

  // Add the Date header to the request.
  RequestBuilder& Date();

private:
  void SetContent(RequestPtr request, std::string&& data);
  
private:
  std::string method_;

  std::string url_;

  // URL query parameters.
  std::vector<std::string> parameters_;

  // Data to send in the body of the request.
  std::string data_;

  // Is the data to send a JSON string?
  bool json_ = false;

  // Files to upload for a POST request.
  std::vector<FormPartPtr> form_parts_;

  // Compress the request content.
  // NOTE: Most servers don't support compressed requests.
  // Even the requests module from Python doesn't have a built-in support.
  // See: https://github.com/kennethreitz/requests/issues/1753
  bool gzip_ = false;

  // Additional request headers.
  std::vector<std::string> headers_;

  // Persistent connection.
  bool keep_alive_ = true;
};

}  // namespace webcc

#endif  // WEBCC_REQUEST_BUILDER_H_
