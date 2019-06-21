#include "webcc/request.h"

#include "webcc/logger.h"
#include "webcc/utility.h"

namespace webcc {

// -----------------------------------------------------------------------------

namespace misc_strings {

// Literal strings can't be used because they have an extra '\0'.

const char CRLF[] = { '\r', '\n' };
const char DOUBLE_DASHES[] = { '-', '-' };

}  // misc_strings

// -----------------------------------------------------------------------------

void Request::Prepare() {
  CreateStartLine();

  if (url_.port().empty()) {
    SetHeader(headers::kHost, url_.host());
  } else {
    SetHeader(headers::kHost, url_.host() + ":" + url_.port());
  }

  if (form_parts_.empty()) {
    Message::Prepare();
    return;
  }

  // Multipart form data.

  // Another choice to generate the boundary is like what Apache does.
  // See: https://stackoverflow.com/a/5686863
  if (boundary_.empty()) {
    boundary_ = utility::RandomUuid();
  }

  SetContentType("multipart/form-data; boundary=" + boundary_);

  Payload data_payload;

  for (auto& part : form_parts_) {
    AddBoundary(data_payload);
    part->Prepare(&data_payload);
  }
  AddBoundary(data_payload, true);

  // Update Content-Length header.
  std::size_t content_length = 0;
  for (auto& buffer : data_payload) {
    content_length += buffer.size();
  }
  SetContentLength(content_length);

  // Prepare start line and headers.
  Message::Prepare();

  // Append payload of content data.
  payload_.insert(payload_.end(), data_payload.begin(), data_payload.end());
}

void Request::CreateStartLine() {
  if (!start_line_.empty()) {
    return;
  }

  if (url_.host().empty()) {
    throw Error{ Error::kSyntaxError, "Host is missing" };
  }

  std::string target = url_.path();
  if (!url_.query().empty()) {
    target += "?";
    target += url_.query();
  }

  start_line_ = method_;
  start_line_ += " ";
  start_line_ += target;
  start_line_ += " HTTP/1.1";
}

void Request::AddBoundary(Payload& payload, bool end) {
  using boost::asio::buffer;

  payload.push_back(buffer(misc_strings::DOUBLE_DASHES));
  payload.push_back(buffer(boundary_));
  if (end) {
    payload.push_back(buffer(misc_strings::DOUBLE_DASHES));
  }
  payload.push_back(buffer(misc_strings::CRLF));
}

}  // namespace webcc
