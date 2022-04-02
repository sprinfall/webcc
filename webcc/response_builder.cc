#include "webcc/response_builder.h"

#include "webcc/base64.h"
#include "webcc/logger.h"
#include "webcc/utility.h"

#if WEBCC_ENABLE_GZIP
#include "webcc/gzip.h"
#endif

namespace webcc {

ResponsePtr ResponseBuilder::operator()() {
  assert(headers_.size() % 2 == 0);

  auto response = std::make_shared<Response>(code_);

  for (std::size_t i = 1; i < headers_.size(); i += 2) {
    response->SetHeader(std::move(headers_[i - 1]), std::move(headers_[i]));
  }

  if (body_) {
    response->SetContentType(media_type_, charset_);

#if WEBCC_ENABLE_GZIP
    if (gzip_) {
      // Don't try to compress the response if the request doesn't accept gzip.
      if (request_ && request_->AcceptEncodingGzip()) {
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

ResponseBuilder& ResponseBuilder::File(const sfs::path& path,
                                       bool infer_media_type,
                                       std::size_t chunk_size) {
  body_.reset(new FileBody{ path, chunk_size });

  if (infer_media_type) {
    media_type_ = media_types::FromExtension(path.extension().string());
  }

  return *this;
}

ResponseBuilder& ResponseBuilder::Header(string_view key, string_view value) {
  headers_.push_back(ToString(key));
  headers_.push_back(ToString(value));
  return *this;
}

ResponseBuilder& ResponseBuilder::Date() {
  headers_.push_back(headers::kDate);
  headers_.push_back(utility::HttpDate());
  return *this;
}

}  // namespace webcc
