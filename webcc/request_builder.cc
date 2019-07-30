#include "webcc/request_builder.h"

#include "webcc/base64.h"
#include "webcc/logger.h"
#include "webcc/utility.h"

#if WEBCC_ENABLE_GZIP
#include "webcc/gzip.h"
#endif

namespace webcc {

RequestPtr RequestBuilder::operator()() {
  assert(headers_.size() % 2 == 0);

  auto request = std::make_shared<Request>(method_);

  request->set_url(std::move(url_));

  for (std::size_t i = 1; i < headers_.size(); i += 2) {
    request->SetHeader(std::move(headers_[i - 1]), std::move(headers_[i]));
  }

  // If no Keep-Alive, explicitly set `Connection` to "Close".
  //if (!keep_alive_) {
    request->SetHeader(headers::kConnection, "Close");
  //}

  if (body_) {
    request->SetContentType(media_type_, charset_);

#if WEBCC_ENABLE_GZIP
    if (gzip_ && body_->Compress()) {
      request->SetHeader(headers::kContentEncoding, "gzip");
    }
#endif
  } else if (!form_parts_.empty()) {
    // Another choice to generate the boundary is like what Apache does.
    // See: https://stackoverflow.com/a/5686863
    auto boundary = utility::RandomUuid();

    request->SetContentType("multipart/form-data; boundary=" + boundary);

    body_ = std::make_shared<FormBody>(form_parts_, boundary);
  }

  if (body_) {
    request->SetBody(body_, true);
  }

  return request;
}

RequestBuilder& RequestBuilder::File(const std::string& name,
                                     const Path& path,
                                     const std::string& media_type) {
  assert(!name.empty());
  form_parts_.push_back(FormPart::NewFile(name, path, media_type));
  return *this;
}

RequestBuilder& RequestBuilder::Form(const std::string& name,
                                     std::string&& data,
                                     const std::string& media_type) {
  assert(!name.empty());
  form_parts_.push_back(FormPart::New(name, std::move(data), media_type));
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

}  // namespace webcc
