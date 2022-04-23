#include "webcc/response_builder.h"

#include "webcc/logger.h"

#if WEBCC_ENABLE_GZIP
#include "webcc/gzip.h"
#endif

namespace webcc {

ResponsePtr ResponseBuilder::operator()() {
  assert(headers_.size() % 2 == 0);

  auto response = std::make_shared<Response>(code_);

  for (std::size_t i = 1; i < headers_.size(); i += 2) {
    response->SetHeader(headers_[i - 1], headers_[i]);
  }

  // If no keep-alive, explicitly set Connection header to "Close".
  if (!keep_alive_) {
    response->SetHeader(headers::kConnection, "Close");
  }  // else: Do nothing!

  if (body_ != nullptr) {
    response->SetContentType(media_type_, charset_);

#if WEBCC_ENABLE_GZIP
    if (gzip_) {
      // Don't try to compress the response if the request doesn't accept gzip.
      if (request_ != nullptr && request_->AcceptEncodingGzip()) {
        if (body_->Compress()) {
          response->SetHeader(headers::kContentEncoding, "gzip");
        }
      }
    }
#endif  // WEBCC_ENABLE_GZIP
  } else {
    // Ensure that the Content-Length header exists if the body is empty.
    // "Content-Length: 0" is required by most HTTP clients (e.g., Chrome).
    body_ = std::make_shared<webcc::Body>();
  }

  response->SetBody(body_, true);

  return response;
}

}  // namespace webcc
