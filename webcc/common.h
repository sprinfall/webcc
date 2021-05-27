#ifndef WEBCC_COMMON_H_
#define WEBCC_COMMON_H_

#include <cassert>
#include <string>
#include <utility>
#include <vector>

#include "webcc/fs.h"
#include "webcc/globals.h"

namespace webcc {

// -----------------------------------------------------------------------------

using Header = std::pair<std::string, std::string>;

class Headers {
public:
  std::size_t size() const {
    return headers_.size();
  }

  bool empty() const {
    return headers_.empty();
  }

  const std::vector<Header>& data() const {
    return headers_;
  }

  bool Set(const std::string& key, const std::string& value);

  bool Set(std::string&& key, std::string&& value);

  bool Has(const std::string& key) const;

  // Get header by index.
  const Header& Get(std::size_t index) const {
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
  std::vector<Header>::iterator Find(const std::string& key);

  std::vector<Header> headers_;
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

  void Reset();

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

class FormPart;
using FormPartPtr = std::shared_ptr<FormPart>;

// A part of the multipart form data.
class FormPart {
public:
  FormPart() = default;

  FormPart(const FormPart&) = delete;
  FormPart& operator=(const FormPart&) = delete;

  // Construct a non-file part.
  // The data will be moved, no file name is needed.
  // The media type is optional. If the data is a JSON string, you can specify
  // media type as "application/json".
  static FormPartPtr New(const std::string& name, std::string&& data,
                         const std::string& media_type = "");

  // Construct a file part.
  // The file name will be extracted from path.
  // The media type, if not provided, will be inferred from file extension.
  static FormPartPtr NewFile(const std::string& name, const fs::path& path,
                             const std::string& media_type = "");

  // API: SERVER
  const std::string& name() const {
    return name_;
  }

  // API: SERVER/PARSER
  void set_name(const std::string& name) {
    name_ = name;
  }

  // API: SERVER
  const std::string& file_name() const {
    return file_name_;
  }

  // API: SERVER/PARSER
  void set_file_name(const std::string& file_name) {
    file_name_ = file_name;
  }

  // API: SERVER
  const std::string& media_type() const {
    return media_type_;
  }

  // API: SERVER
  const std::string& data() const {
    return data_;
  }

  // API: SERVER/PARSER
  void AppendData(const std::string& data) {
    data_.append(data);
  }

  // API: SERVER/PARSER
  void AppendData(const char* data, std::size_t count) {
    data_.append(data, count);
  }

  // API: CLIENT
  void Prepare(Payload* payload);

  // Free the memory of the data.
  void Free();

  // Get the size of the whole payload.
  // Used by the request to calculate content length.
  std::size_t GetSize();

  // Get the size of the data.
  std::size_t GetDataSize();

  // Dump to output stream for logging purpose.
  void Dump(std::ostream& os, const std::string& prefix) const;

private:
  // Generate headers from properties.
  void SetHeaders();

private:
  // The <input> name within the original HTML form.
  // E.g., given HTML form:
  //   <input name="file1" type="file">
  // the name will be "file1".
  std::string name_;

  // The path of the file to post.
  fs::path path_;

  // The original local file name.
  // E.g., "baby.jpg".
  std::string file_name_;

  // The content-type if the media type is known (e.g., inferred from the file
  // extension or operating system typing information) or as
  // application/octet-stream.
  // E.g., "image/jpeg".
  std::string media_type_;

  // Headers generated from the above properties.
  // Only Used to prepare payload.
  Headers headers_;

  std::string data_;
};

}  // namespace webcc

#endif  // WEBCC_COMMON_H_
