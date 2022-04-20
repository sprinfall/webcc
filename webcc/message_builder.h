#ifndef WEBCC_MESSAGE_BUILDER_H_
#define WEBCC_MESSAGE_BUILDER_H_

#include <string>
#include <vector>

#include "webcc/body.h"
#include "webcc/message.h"
#include "webcc/utility.h"  // for utility::HttpDate

namespace webcc {

template <typename SubBuilder>
class MessageBuilder {
public:
  MessageBuilder(const MessageBuilder&) = delete;
  MessageBuilder& operator=(const MessageBuilder&) = delete;

  virtual ~MessageBuilder() = default;

  SubBuilder& MediaType(std::string_view media_type) {
    media_type_ = media_type;
    return *sub_this_;
  }

  SubBuilder& Charset(std::string_view charset) {
    charset_ = charset;
    return *sub_this_;
  }

  // Set Media Type to "application/json".
  SubBuilder& Json() {
    media_type_ = media_types::kApplicationJson;
    return *sub_this_;
  }

  // Set Charset to "utf-8".
  SubBuilder& Utf8() {
    charset_ = charsets::kUtf8;
    return *sub_this_;
  }

  SubBuilder& Header(std::string_view key, std::string_view value) {
    headers_.emplace_back(key);
    headers_.emplace_back(value);
    return *sub_this_;
  }

  // Add the Date header to the message.
  SubBuilder& Date() {
    headers_.emplace_back(headers::kDate);
    headers_.emplace_back(utility::HttpDate());
    return *sub_this_;
  }

  SubBuilder& KeepAlive(bool keep_alive = true) {
    keep_alive_ = keep_alive;
    return *sub_this_;
  }

#if WEBCC_ENABLE_GZIP
  // Compress the body data (only for string body).
  // NOTE:
  // Most servers don't support compressed requests.
  // Even Python `requests` doesn't have a built-in support.
  // See: https://github.com/kennethreitz/requests/issues/1753
  SubBuilder& Gzip(bool gzip = true) {
    gzip_ = gzip;
    return *sub_this_;
  }
#endif  // WEBCC_ENABLE_GZIP

  SubBuilder& Body(const std::string& data) {
    body_.reset(new StringBody{ data, false });
    return *sub_this_;
  }

  SubBuilder& Body(std::string&& data) {
    body_.reset(new StringBody{ std::move(data), false });
    return *sub_this_;
  }

  // Use the file content as the body.
  // NOTE: error_codes::kFileError might be thrown.
  SubBuilder& File(const sfs::path& path, bool infer_media_type = true,
                   std::size_t chunk_size = 1024) {
    body_.reset(new FileBody{ path, chunk_size });
    if (infer_media_type) {
      media_type_ = media_types::FromExtension(path.extension().string());
    }
    return *sub_this_;
  }

protected:
  MessageBuilder(SubBuilder* sub_this) : sub_this_(sub_this) {
  }

  SubBuilder* sub_this_;

  // The media (or MIME) type of the Content-Type header.
  // E.g., "application/json".
  std::string media_type_;

  // The charset of the Content-Type header.
  // E.g., "utf-8".
  std::string charset_;

  // Persistent connection.
  bool keep_alive_ = true;

#if WEBCC_ENABLE_GZIP
  // Compress the body data (only for string body).
  bool gzip_ = false;
#endif

  // Additional headers with the following sequence:
  //   { key1, value1, key2, value2, ... }
  std::vector<std::string> headers_;

  BodyPtr body_;
};

}  // namespace webcc

#endif  // WEBCC_MESSAGE_BUILDER_H_
