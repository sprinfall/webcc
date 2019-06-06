#include "webcc/request_builder.h"

#include "webcc/base64.h"
#include "webcc/logger.h"
#include "webcc/utility.h"

#if WEBCC_ENABLE_GZIP
#include "webcc/gzip.h"
#endif

namespace webcc {

RequestPtr RequestBuilder::operator()() {
  assert(parameters_.size() % 2 == 0);
  assert(headers_.size() % 2 == 0);

  auto request = std::make_shared<Request>(method_, url_);

  for (std::size_t i = 1; i < parameters_.size(); i += 2) {
    request->AddQuery(parameters_[i - 1], parameters_[i]);
  }

  for (std::size_t i = 1; i < headers_.size(); i += 2) {
    request->SetHeader(std::move(headers_[i - 1]), std::move(headers_[i]));
  }

  // No keep-alive?
  if (!keep_alive_) {
    request->SetHeader(headers::kConnection, "Close");
  }

  if (!data_.empty()) {
    SetContent(request, std::move(data_));

    // TODO: Request-level charset.
    if (json_) {
      request->SetContentType(media_types::kApplicationJson, "");
    }
  } else if (!form_parts_.empty()) {
    request->set_form_parts(std::move(form_parts_));
  }

  return request;
}

RequestBuilder& RequestBuilder::File(const std::string& name,
                                     const Path& path,
                                     const std::string& media_type) {
  assert(!name.empty());
  auto part = std::make_shared<FormPart>(name, path, media_type);
  form_parts_.push_back(part);
  return *this;
}

RequestBuilder& RequestBuilder::Form(const std::string& name,
                                     std::string&& data,
                                     const std::string& media_type) {
  assert(!name.empty());
  auto part = std::make_shared<FormPart>(name, std::move(data), media_type);
  form_parts_.push_back(part);
  return *this;
}

RequestBuilder& RequestBuilder::Auth(const std::string& type,
                                     const std::string& credentials) {
  headers_.push_back(headers::kAuthorization);
  headers_.push_back(type + " " + credentials);
  return *this;
}

RequestBuilder& RequestBuilder::AuthBasic(const std::string& login,
                                          const std::string& password) {
  auto credentials = Base64Encode(login + ":" + password);
  return Auth("Basic", credentials);
}

RequestBuilder& RequestBuilder::AuthToken(const std::string& token) {
  return Auth("Token", token);
}

RequestBuilder& RequestBuilder::Date() {
  headers_.push_back(headers::kDate);
  headers_.push_back(utility::GetTimestamp());
  return *this;
}

void RequestBuilder::SetContent(RequestPtr request, std::string&& data) {
#if WEBCC_ENABLE_GZIP
  if (gzip_ && data.size() > kGzipThreshold) {
    std::string compressed;
    if (gzip::Compress(data, &compressed)) {
      request->SetContent(std::move(compressed), true);
      request->SetHeader(headers::kContentEncoding, "gzip");
      return;
    }

    LOG_WARN("Cannot compress the content data!");
  }
#endif  // WEBCC_ENABLE_GZIP

  request->SetContent(std::move(data), true);
}

}  // namespace webcc
