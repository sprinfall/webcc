#include "webcc/http_request_builder.h"

#include "webcc/logger.h"
#include "webcc/zlib_wrapper.h"

namespace webcc {

HttpRequestPtr HttpRequestBuilder::operator()() {
  assert(parameters_.size() % 2 == 0);
  assert(headers_.size() % 2 == 0);

  auto request = std::make_shared<HttpRequest>(method_, url_);

  for (std::size_t i = 1; i < parameters_.size(); i += 2) {
    request->AddParameter(parameters_[i - 1], parameters_[i]);
  }

  for (std::size_t i = 1; i < headers_.size(); i += 2) {
    request->SetHeader(std::move(headers_[i - 1]), std::move(headers_[i]));
  }

  // No keep-alive?
  if (!keep_alive_) {
    request->SetHeader(http::headers::kConnection, "Close");
  }

  if (!data_.empty()) {
    if (gzip_ && data_.size() > kGzipThreshold) {
      std::string compressed;
      if (Compress(data_, &compressed)) {
        request->SetContent(std::move(compressed), true);
        request->SetHeader(http::headers::kContentEncoding, "gzip");
      } else {
        LOG_WARN("Cannot compress the content data!");
        request->SetContent(std::move(data_), true);
      }
    } else {
      request->SetContent(std::move(data_), true);
    }

    // TODO: Request-level charset.
    if (json_) {
      request->SetContentType(http::media_types::kApplicationJson, "");
    }
  }

  return request;
}

}  // namespace webcc
