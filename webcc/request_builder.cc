#include "webcc/request_builder.h"

#include "webcc/logger.h"
#include "webcc/string.h"  // for RandomAsciiString

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

  // If no keep-alive, explicitly set Connection header to "Close".
  if (!keep_alive_) {
    request->SetHeader(headers::kConnection, "Close");
  }  // else: Do nothing!

  if (body_ != nullptr) {
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

  if (body_ != nullptr) {
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

}  // namespace webcc
