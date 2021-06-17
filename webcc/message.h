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

class Message {
public:
  Message();

  Message(const Message&) = delete;
  Message& operator=(const Message&) = delete;

  virtual ~Message() = default;

  const std::string& start_line() const {
    return start_line_;
  }

  void set_start_line(string_view start_line) {
    start_line_ = ToString(start_line);
  }

  void SetHeader(Header&& header) {
    headers_.Set(std::move(header.first), std::move(header.second));
  }

  void SetHeader(string_view key, string_view value) {
    headers_.Set(key, value);
  }

  const std::string& GetHeader(string_view key, bool* existed = nullptr) const {
    return headers_.Get(key, existed);
  }

  bool HasHeader(string_view key) const {
    return headers_.Has(key);
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
  const std::string& data() const;

  // Get the body as a FileBody.
  // Return null if the body is not a FileBody.
  std::shared_ptr<FileBody> file_body() const;

  // Check `Connection` header to see if it's "Keep-Alive".
  bool IsConnectionKeepAlive() const;

  // Determine content encoding (gzip, deflate or unknown) from
  // `Content-Encoding` header.
  ContentEncoding GetContentEncoding() const;

  // Check `Accept-Encoding` header to see if it contains "gzip".
  bool AcceptEncodingGzip() const;

  // Set `Content-Type` header. E.g.,
  //   SetContentType("application/json; charset=utf-8")
  void SetContentType(string_view content_type) {
    SetHeader(headers::kContentType, content_type);
  }

  // Set `Content-Type` header. E.g.,
  //   SetContentType("application/json", "utf-8")
  void SetContentType(string_view media_type, string_view charset);

  // Make the message complete in order to be sent.
  virtual void Prepare() = 0;

  // Get the payload for the socket to write.
  // This doesn't include the payload(s) of the body!
  Payload GetPayload() const;

  // Dump to output stream for logging purpose.
  void Dump(std::ostream& os) const;

  // Dump to string for logging purpose.
  std::string Dump() const;

protected:
  BodyPtr body_;

  Headers headers_;

  std::string start_line_;

  std::size_t content_length_ = kInvalidLength;
};

}  // namespace webcc

#endif  // WEBCC_MESSAGE_H_
