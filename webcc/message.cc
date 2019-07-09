#include "webcc/message.h"

#include <sstream>

#include "boost/algorithm/string.hpp"

#include "webcc/logger.h"
#include "webcc/utility.h"

namespace webcc {

Message::Message() : body_(new Body{}), content_length_(kInvalidLength) {
}

void Message::SetBody(BodyPtr body, bool set_length) {
  if (body == body_) {
    return;
  }

  if (!body) {
    body_.reset(new Body{});
  } else {
    body_ = body;
  }

  if (set_length) {
    content_length_ = body_->GetSize();
    SetHeader(headers::kContentLength, std::to_string(content_length_));
  }
}

const std::string& Message::data() const {
  static const std::string kEmptyData;

  auto string_body = std::dynamic_pointer_cast<StringBody>(body_);

  if (string_body) {
    return string_body->data();
  }

  return kEmptyData;
}

bool Message::IsConnectionKeepAlive() const {
  using headers::kConnection;

  bool existed = false;
  const std::string& connection = GetHeader(kConnection, &existed);

  if (!existed) {
    // Keep-Alive is by default for HTTP/1.1.
    return true;
  }

  if (boost::iequals(connection, "Keep-Alive")) {
    return true;
  }

  return false;
}

ContentEncoding Message::GetContentEncoding() const {
  using headers::kContentEncoding;

  const std::string& encoding = GetHeader(kContentEncoding);

  if (encoding == "gzip") {
    return ContentEncoding::kGzip;
  }

  if (encoding == "deflate") {
    return ContentEncoding::kDeflate;
  }

  return ContentEncoding::kUnknown;
}

bool Message::AcceptEncodingGzip() const {
  using headers::kAcceptEncoding;

  return GetHeader(kAcceptEncoding).find("gzip") != std::string::npos;
}

void Message::SetContentType(const std::string& media_type,
                             const std::string& charset) {
  using headers::kContentType;

  if (charset.empty()) {
    SetHeader(kContentType, media_type);
  } else {
    SetHeader(kContentType, media_type + "; charset=" + charset);
  }
}

Payload Message::GetPayload() const {
  using boost::asio::buffer;

  Payload payload;

  payload.push_back(buffer(start_line_));
  payload.push_back(buffer(literal_buffers::CRLF));

  for (const Header& h : headers_.data()) {
    payload.push_back(buffer(h.first));
    payload.push_back(buffer(literal_buffers::HEADER_SEPARATOR));
    payload.push_back(buffer(h.second));
    payload.push_back(buffer(literal_buffers::CRLF));
  }

  payload.push_back(buffer(literal_buffers::CRLF));

  return payload;
}

void Message::Dump(std::ostream& os) const {
  const std::string prefix = "    > ";

  os << prefix << start_line_ << std::endl;

  for (const Header& h : headers_.data()) {
    os << prefix << h.first << ": " << h.second << std::endl;
  }

  os << prefix << std::endl;

  body_->Dump(os, prefix);
}

std::string Message::Dump() const {
  std::stringstream ss;
  Dump(ss);
  return ss.str();
}

}  // namespace webcc
