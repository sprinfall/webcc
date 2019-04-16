#ifndef WEBCC_COMMON_H_
#define WEBCC_COMMON_H_

#include <cassert>
#include <string>
#include <utility>
#include <vector>

#include "boost/asio/buffer.hpp"  // for const_buffer
#include "boost/filesystem/path.hpp"

#include "webcc/globals.h"

namespace webcc {

// -----------------------------------------------------------------------------

using Path = boost::filesystem::path;

using Payload = std::vector<boost::asio::const_buffer>;

// -----------------------------------------------------------------------------

// Split a string to two parts by the given token.
bool Split2(const std::string& str, char token, std::string* part1,
            std::string* part2);

// Read entire file into string.
bool ReadFile(const Path& path, std::string* output);

// -----------------------------------------------------------------------------

typedef std::pair<std::string, std::string> HttpHeader;

class HttpHeaders {
public:
  std::size_t size() const {
    return headers_.size();
  }

  bool empty() const {
    return headers_.empty();
  }

  const std::vector<HttpHeader>& data() const {
    return headers_;
  }

  void Set(const std::string& key, const std::string& value);

  void Set(std::string&& key, std::string&& value);

  bool Has(const std::string& key) const;

  // Get header by index.
  const HttpHeader& Get(std::size_t index) const {
    assert(index < size());
    return headers_[index];
  }

  // Get header value by key.
  // If there's no such header with the given key, besides return empty, the
  // optional |existed| parameter will be set to false.
  const std::string& Get(const std::string& key, bool* existed = nullptr) const;

  void Clear() {
    headers_.clear();
  }

private:
  std::vector<HttpHeader>::iterator Find(const std::string& key);

  std::vector<HttpHeader> headers_;
};

// -----------------------------------------------------------------------------

// Content-Type header.
// Syntax:
//   Content-Type: text/html; charset=utf-8
//   Content-Type: multipart/form-data; boundary=something
class ContentType {
public:
  explicit ContentType(const std::string& str = "");

  void Parse(const std::string& str);

  bool Valid() const;

  bool multipart() const {
    return multipart_;
  }

  const std::string& media_type() const {
    return media_type_;
  }

  const std::string& charset() const {
    assert(!multipart_);
    return additional_;
  }

  const std::string& boundary() const {
    assert(multipart_);
    return additional_;
  }

private:
  void Init(const std::string& str);

private:
  std::string media_type_;
  std::string additional_;
  bool multipart_ = false;
};

// -----------------------------------------------------------------------------

// Content-Disposition header.
// Syntax:
//   Content-Disposition: form-data
//   Content-Disposition: form-data; name="fieldName"
//   Content-Disposition: form-data; name="fieldName"; filename="filename.jpg"
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Content-Disposition
class ContentDisposition {
public:
  explicit ContentDisposition(const std::string& str) {
    valid_ = Init(str);
  }

  bool valid() const {
    return valid_;
  }

  const std::string& name() const {
    return name_;
  }

  const std::string& file_name() const {
    return file_name_;
  }

private:
  bool Init(const std::string& str);

private:
  std::string name_;
  std::string file_name_;
  bool valid_ = false;
};

// -----------------------------------------------------------------------------

// Form data part.
class FormPart {
public:
  FormPart() = default;

  FormPart(const std::string& name, const Path& path,
           const std::string& mime_type = "");

  FormPart(const std::string& name, std::string&& data,
           const std::string& mime_type = "");

#if WEBCC_DEFAULT_MOVE_COPY_ASSIGN

  FormPart(FormPart&&) = default;
  FormPart& operator=(FormPart&&) = default;

#else

  FormPart(FormPart&& rhs)
      : name_(std::move(rhs.name_)),
        file_name_(std::move(rhs.file_name_)),
        mime_type_(std::move(rhs.mime_type_)),
        data_(std::move(rhs.data_)) {
  }

  FormPart& operator=(FormPart&& rhs) {
    if (&rhs != this) {
      name_ = std::move(rhs.name_);
      file_name_ = std::move(rhs.file_name_);
      mime_type_ = std::move(rhs.mime_type_);
      data_ = std::move(rhs.data_);
    }
    return *this;
  }

#endif  // WEBCC_DEFAULT_MOVE_COPY_ASSIGN

  const std::string& name() const {
    return name_;
  }

  void set_name(const std::string& name) {
    name_ = name;
  }

  const std::string& file_name() const {
    return file_name_;
  }

  void set_file_name(const std::string& file_name) {
    file_name_ = file_name;
  }

  const std::string& mime_type() const {
    return mime_type_;
  }

  const std::string& data() const {
    return data_;
  }

  void AppendData(const std::string& data) {
    data_.append(data);
  }

  void AppendData(const char* data, std::size_t size) {
    data_.append(data, size);
  }

  void Prepare(Payload& payload);

private:
  std::string name_;

  // E.g., example.jpg
  // TODO: Unicode
  std::string file_name_;

  // E.g., image/jpeg
  std::string mime_type_;

  HttpHeaders headers_;

  // Binary file data.
  std::string data_;
};

}  // namespace webcc

#endif  // WEBCC_COMMON_H_
