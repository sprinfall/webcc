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

  auto request = std::make_shared<Response>(code_);

  for (std::size_t i = 1; i < headers_.size(); i += 2) {
    request->SetHeader(std::move(headers_[i - 1]), std::move(headers_[i]));
  }

  if (!data_.empty()) {
    SetContent(request, std::move(data_));

    // TODO: charset.
    if (json_) {
      request->SetContentType(media_types::kApplicationJson, "");
    }
  }

  return request;
}

ResponseBuilder& ResponseBuilder::Date() {
  headers_.push_back(headers::kDate);
  headers_.push_back(utility::GetTimestamp());
  return *this;
}

void ResponseBuilder::SetContent(ResponsePtr response, std::string&& data) {
#if WEBCC_ENABLE_GZIP
  if (gzip_ && data.size() > kGzipThreshold) {
    std::string compressed;
    if (gzip::Compress(data, &compressed)) {
      response->SetContent(std::move(compressed), true);
      response->SetHeader(headers::kContentEncoding, "gzip");
      return;
    }

    LOG_WARN("Cannot compress the content data!");
  }
#endif  // WEBCC_ENABLE_GZIP

  response->SetContent(std::move(data), true);
}

}  // namespace webcc
