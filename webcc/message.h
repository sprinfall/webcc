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

  virtual ~Message() = default;

  // ---------------------------------------------------------------------------

  void SetBody(BodyPtr body, bool set_length);

  BodyPtr body() const {
    return body_;
  }

  // Get the data from the string body.
  // Exception Error(kDataError) will be thrown if the body is FormBody.
  const std::string& data() const;

  // ---------------------------------------------------------------------------

  void SetHeader(Header&& header) {
    headers_.Set(std::move(header.first), std::move(header.second));
  }

  void SetHeader(const std::string& key, const std::string& value) {
    headers_.Set(key, value);
  }

  const std::string& GetHeader(const std::string& key,
                               bool* existed = nullptr) const {
    return headers_.Get(key, existed);
  }

  bool HasHeader(const std::string& key) const {
    return headers_.Has(key);
  }

  // ---------------------------------------------------------------------------

  const std::string& start_line() const {
    return start_line_;
  }

  void set_start_line(const std::string& start_line) {
    start_line_ = start_line;
  }

  std::size_t content_length() const {
    return content_length_;
  }

  void set_content_length(std::size_t content_length) {
    content_length_ = content_length;
  }

  // ---------------------------------------------------------------------------

  // Check `Connection` header to see if it's "Keep-Alive".
  bool IsConnectionKeepAlive() const;

  // Determine content encoding (gzip, deflate or unknown) from
  // `Content-Encoding` header.
  ContentEncoding GetContentEncoding() const;

  // Check `Accept-Encoding` header to see if it contains "gzip".
  bool AcceptEncodingGzip() const;

  // Set `Content-Type` header. E.g.,
  //   SetContentType("application/json; charset=utf-8")
  void SetContentType(const std::string& content_type) {
    SetHeader(headers::kContentType, content_type);
  }

  // Set `Content-Type` header. E.g.,
  //   SetContentType("application/json", "utf-8")
  void SetContentType(const std::string& media_type,
                      const std::string& charset);

  // ---------------------------------------------------------------------------

  // Make the message complete in order to be sent.
  virtual void Prepare() = 0;

  // Get the payload for the socket to write.
  // This doesn't include the payload(s) of the body!
  Payload GetPayload() const;

  // ---------------------------------------------------------------------------

  // Dump to output stream for logging purpose.
  void Dump(std::ostream& os) const;

  // Dump to string for logging purpose.
  std::string Dump() const;

protected:
  BodyPtr body_;

  Headers headers_;

  std::string start_line_;

  std::size_t content_length_;
};

}  // namespace webcc

#endif  // WEBCC_MESSAGE_H_
