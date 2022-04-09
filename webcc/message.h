#ifndef WEBCC_MESSAGE_H_
#define WEBCC_MESSAGE_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "webcc/body.h"
#include "webcc/common.h"
#include "webcc/globals.h"

namespace webcc {

namespace log_prefix {
const char* const kIncoming = "    < ";
const char* const kOutgoing = "    > ";
}

class Message {
public:
  Message();

  Message(const Message&) = delete;
  Message& operator=(const Message&) = delete;

  virtual ~Message() = default;

  const std::string& start_line() const {
    return start_line_;
  }

  void set_start_line(std::string_view start_line) {
    start_line_ = start_line;
  }

  void SetHeader(Header&& header) {
    headers_.Set(std::move(header.first), std::move(header.second));
  }

  void SetHeader(std::string_view key, std::string_view value) {
    headers_.Set(key, value);
  }

  std::string_view GetHeader(std::string_view key) const {
    return headers_.Get(key);
  }

  bool HeaderExist(std::string_view key) const {
    return !headers_.Get(key).empty();
  }

  std::size_t content_length() const {
    return content_length_;
  }

  void set_content_length(std::size_t content_length) {
    content_length_ = content_length;
  }

  void SetBody(BodyPtr body, bool set_length);

  BodyPtr body() const {
    return body_;
  }

  // Get the data from the (string) body.
  // Return empty string if the body is not a StringBody.
  // TODO: Return string_view?
  const std::string& data() const;

  // Get the body as a FileBody.
  // Return null if the body is not a FileBody.
  std::shared_ptr<FileBody> file_body() const {
    return std::dynamic_pointer_cast<FileBody>(body_);
  }

  // Check the Connection header to see if it's "Keep-Alive".
  bool IsConnectionKeepAlive() const;

  // Determine content encoding (gzip, deflate or unknown) from
  // the Content-Encoding header.
  ContentEncoding GetContentEncoding() const;

  // Check the Accept-Encoding header to see if it contains "gzip".
  bool AcceptEncodingGzip() const {
    return GetHeader(headers::kAcceptEncoding).find("gzip") !=
           std::string_view::npos;
  }

  // Set the Content-Type header.
  // E.g. SetContentType("application/json; charset=utf-8")
  void SetContentType(std::string_view content_type) {
    SetHeader(headers::kContentType, content_type);
  }

  // Set the Content-Type header.
  // E.g., SetContentType("application/json", "utf-8")
  void SetContentType(std::string_view media_type, std::string_view charset);

  // Make the message complete in order to be sent.
  virtual void Prepare() = 0;

  // Get the payload for the socket to write.
  // This doesn't include the payload(s) of the body!
  Payload GetPayload() const;

  // Dump to output stream for logging purpose.
  void Dump(std::ostream& os, std::string_view prefix = "") const;

  // Dump to string for logging purpose.
  std::string Dump(std::string_view prefix = "") const;

protected:
  std::string start_line_;

  Headers headers_;

  std::size_t content_length_ = kInvalidLength;

  BodyPtr body_;
};

}  // namespace webcc

#endif  // WEBCC_MESSAGE_H_
