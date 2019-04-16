#include "webcc/http_request_builder.h"

#include "webcc/base64.h"
#include "webcc/logger.h"
#include "webcc/utility.h"
#include "webcc/zlib_wrapper.h"

namespace webcc {

HttpRequestPtr HttpRequestBuilder::Build() {
  assert(parameters_.size() % 2 == 0);
  assert(headers_.size() % 2 == 0);

  auto request = std::make_shared<HttpRequest>(method_, url_);

  for (std::size_t i = 1; i < parameters_.size(); i += 2) {
    request->AddQuery(parameters_[i - 1], parameters_[i]);
  }

  for (std::size_t i = 1; i < headers_.size(); i += 2) {
    request->SetHeader(std::move(headers_[i - 1]), std::move(headers_[i]));
  }

  // No keep-alive?
  if (!keep_alive_) {
    request->SetHeader(http::headers::kConnection, "Close");
  }

  if (!data_.empty()) {
    SetContent(request, std::move(data_));

    // TODO: Request-level charset.
    if (json_) {
      request->SetContentType(http::media_types::kApplicationJson, "");
    }
  } else if (!form_parts_.empty()) {
    request->set_form_parts(std::move(form_parts_));
  }

  return request;
}

HttpRequestBuilder& HttpRequestBuilder::File(const std::string& name,
                                             const Path& path,
                                             const std::string& mime_type) {
  assert(!name.empty());
  form_parts_.push_back(FormPart{ name, path, mime_type });
  return *this;
}

HttpRequestBuilder& HttpRequestBuilder::Form(const std::string& name,
                                             std::string&& data,
                                             const std::string& mime_type) {
  assert(!name.empty());
  form_parts_.push_back(FormPart{ name, std::move(data), mime_type });
  return *this;
}

HttpRequestBuilder& HttpRequestBuilder::Auth(const std::string& type,
                                             const std::string& credentials) {
  headers_.push_back(http::headers::kAuthorization);
  headers_.push_back(type + " " + credentials);
  return *this;
}

HttpRequestBuilder& HttpRequestBuilder::AuthBasic(const std::string& login,
                                                  const std::string& password) {
  auto credentials = Base64Encode(login + ":" + password);
  return Auth("Basic", credentials);
}

HttpRequestBuilder& HttpRequestBuilder::AuthToken(const std::string& token) {
  return Auth("Token", token);
}

void HttpRequestBuilder::SetContent(HttpRequestPtr request,
                                    std::string&& data) {
  if (gzip_ && data.size() > kGzipThreshold) {
    std::string compressed;
    if (Compress(data, &compressed)) {
      request->SetContent(std::move(compressed), true);
      request->SetHeader(http::headers::kContentEncoding, "gzip");
      return;
    }

    LOG_WARN("Cannot compress the content data!");
  }

  request->SetContent(std::move(data), true);
}

}  // namespace webcc
