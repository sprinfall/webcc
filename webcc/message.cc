#include "webcc/message.h"

#include <sstream>

#include "boost/algorithm/string.hpp"

#include "webcc/logger.h"
#include "webcc/utility.h"

namespace webcc {

Message::Message() : body_(new Body{}) {
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
  if (!string_body) {
    return kEmptyData;
  }
  return string_body->data();
}

std::shared_ptr<FileBody> Message::file_body() const {
  return std::dynamic_pointer_cast<FileBody>(body_);
}

bool Message::IsConnectionKeepAlive() const {
  bool existed = false;
  const auto& connection = GetHeader(headers::kConnection, &existed);

  if (!existed) {
    // Keep-Alive is by default for HTTP/1.1.
    return true;
  }

  return boost::iequals(connection, "Keep-Alive");
}

ContentEncoding Message::GetContentEncoding() const {
  const auto& encoding = GetHeader(headers::kContentEncoding);

  if (encoding == "gzip") {
    return ContentEncoding::kGzip;
  } else if (encoding == "deflate") {
    return ContentEncoding::kDeflate;
  } else {
    return ContentEncoding::kUnknown;
  }
}

bool Message::AcceptEncodingGzip() const {
  return GetHeader(headers::kAcceptEncoding).find("gzip") != std::string::npos;
}

void Message::SetContentType(string_view media_type, string_view charset) {
  if (!media_type.empty()) {
    if (charset.empty()) {
      SetHeader(headers::kContentType, media_type);
    } else {
      std::string value = ToString(media_type);
      value += "; charset=";
      value += ToString(charset);
      SetHeader(headers::kContentType, value);
    }
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
  static const char* const s_prefix = "    > ";

  os << s_prefix << start_line_ << std::endl;

  for (const Header& h : headers_.data()) {
    os << s_prefix << h.first << ": " << h.second << std::endl;
  }

  os << s_prefix << std::endl;

  body_->Dump(os, s_prefix);
}

std::string Message::Dump() const {
  std::ostringstream ss;
  Dump(ss);
  return ss.str();
}

}  // namespace webcc
