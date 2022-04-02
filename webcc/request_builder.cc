#include "webcc/request_builder.h"

#include "webcc/base64.h"
#include "webcc/logger.h"
#include "webcc/string.h"
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
    request->SetHeader(headers_[i - 1], headers_[i]);
  }

  // If no Keep-Alive, explicitly set `Connection` to "Close".
  if (!keep_alive_) {
    request->SetHeader(headers::kConnection, "Close");
  }

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
    std::string boundary = RandomAsciiString(30);

    request->SetContentType("multipart/form-data; boundary=" + boundary);

    body_ = std::make_shared<FormBody>(form_parts_, boundary);
  }

  if (body_) {
    request->SetBody(body_, true);
  }

  return request;
}

#if WEBCC_ENABLE_GZIP

// Accept Gzip compressed response data or not.
RequestBuilder& RequestBuilder::AcceptGzip(bool gzip) {
  if (gzip) {
    return Header(headers::kAcceptEncoding, "gzip, deflate");
  } else {
    return Header(headers::kAcceptEncoding, "identity");
  }
}

#endif  // WEBCC_ENABLE_GZIP

RequestBuilder& RequestBuilder::File(const fs::path& path,
                                     bool infer_media_type,
                                     std::size_t chunk_size) {
  body_.reset(new FileBody{ path, chunk_size });

  if (infer_media_type) {
    media_type_ = media_types::FromExtension(path.extension().string());
  }

  return *this;
}

RequestBuilder& RequestBuilder::FormFile(string_view name,
                                         const fs::path& path,
                                         string_view media_type) {
  assert(!name.empty());
  return Form(FormPart::NewFile(name, path, media_type));
}

RequestBuilder& RequestBuilder::FormData(string_view name,
                                         std::string&& data,
                                         string_view media_type) {
  assert(!name.empty());
  return Form(FormPart::New(name, std::move(data), media_type));
}

RequestBuilder& RequestBuilder::Header(string_view key, string_view value) {
  headers_.push_back(ToString(key));
  headers_.push_back(ToString(value));
  return *this;
}

RequestBuilder& RequestBuilder::Auth(string_view type,
                                     string_view credentials) {
  headers_.push_back(headers::kAuthorization);
  headers_.push_back(ToString(type) + " " + ToString(credentials));
  return *this;
}

RequestBuilder& RequestBuilder::AuthBasic(string_view login,
                                          string_view password) {
  auto credentials =
      Base64Encode(ToString(login) + ":" + ToString(password));
  return Auth("Basic", credentials);
}

RequestBuilder& RequestBuilder::AuthToken(string_view token) {
  return Auth("Token", token);
}

RequestBuilder& RequestBuilder::Date() {
  headers_.push_back(headers::kDate);
  headers_.push_back(utility::HttpDate());
  return *this;
}

}  // namespace webcc
