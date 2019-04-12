#include "webcc/http_request.h"

#include "webcc/logger.h"
#include "webcc/utility.h"

namespace webcc {

// -----------------------------------------------------------------------------

namespace misc_strings {

const char CRLF[] = { '\r', '\n' };
const char DOUBLE_DASHES[] = { '-', '-' };

}  // misc_strings

// -----------------------------------------------------------------------------

void HttpRequest::Prepare() {
  CreateStartLine();

  if (url_.port().empty()) {
    SetHeader(http::headers::kHost, url_.host());
  } else {
    SetHeader(http::headers::kHost, url_.host() + ":" + url_.port());
  }

  if (form_parts_.empty()) {
    HttpMessage::Prepare();
  } else {
    // Multipart form data.

    // Another choice to generate the boundary is what Apache does.
    // See: https://stackoverflow.com/a/5686863
    if (boundary_.empty()) {
      boundary_ = RandomUuid();
    }

    SetContentType("multipart/form-data; boundary=" + boundary_);

    Payload data_payload;

    using boost::asio::buffer;

    for (auto& part : form_parts_) {
      // Boundary
      data_payload.push_back(buffer(misc_strings::DOUBLE_DASHES));
      data_payload.push_back(buffer(boundary_));
      data_payload.push_back(buffer(misc_strings::CRLF));

      part.Prepare(data_payload);
    }

    // Boundary end
    data_payload.push_back(buffer(misc_strings::DOUBLE_DASHES));
    data_payload.push_back(buffer(boundary_));
    data_payload.push_back(buffer(misc_strings::DOUBLE_DASHES));
    data_payload.push_back(buffer(misc_strings::CRLF));

    // Update Content-Length header.
    std::size_t content_length = 0;
    for (auto& buffer : data_payload) {
      content_length += buffer.size();
    }
    SetContentLength(content_length);

    // Prepare start line and headers.
    HttpMessage::Prepare();

    // Append payload of content data.
    payload_.insert(payload_.end(), data_payload.begin(), data_payload.end());
  }
}

void HttpRequest::CreateStartLine() {
  if (!start_line_.empty()) {
    return;
  }

  if (url_.host().empty()) {
    throw Exception(kSchemaError, "Invalid request: host is missing.");
  }

  std::string target = "/" + url_.path();
  if (!url_.query().empty()) {
    target += "?";
    target += url_.query();
  }

  start_line_ = method_;
  start_line_ += " ";
  start_line_ += target;
  start_line_ += " HTTP/1.1";
}

}  // namespace webcc
