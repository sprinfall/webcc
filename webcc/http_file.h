#ifndef WEBCC_HTTP_FILE_H_
#define WEBCC_HTTP_FILE_H_

#include <string>

#include "boost/filesystem/path.hpp"

#include "webcc/globals.h"

namespace webcc {

using Path = boost::filesystem::path;

// File for HTTP transfer (upload/download).
class HttpFile {
public:
  HttpFile() = default;

  explicit HttpFile(const Path& path, const std::string& mime_type = "");

  HttpFile(std::string&& data, const std::string& file_name,
           const std::string& mime_type = "");

#if WEBCC_DEFAULT_MOVE_COPY_ASSIGN

  HttpFile(HttpFile&&) = default;
  HttpFile& operator=(HttpFile&&) = default;

#else

  HttpFile(HttpFile&& rhs)
      : data_(std::move(rhs.data_)),
        file_name_(std::move(rhs.file_name_)),
        mime_type_(std::move(rhs.mime_type_)) {
  }

  HttpFile& operator=(HttpFile&& rhs) {
    if (&rhs != this) {
      data_ = std::move(rhs.data_);
      file_name_ = std::move(rhs.file_name_);
      mime_type_ = std::move(rhs.mime_type_);
    }
    return *this;
  }

#endif  // WEBCC_DEFAULT_MOVE_COPY_ASSIGN

  const std::string& data() const {
    return data_;
  }

  const std::string& file_name() const {
    return file_name_;
  }

  const std::string& mime_type() const {
    return mime_type_;
  }

private:
  // Binary file data.
  // TODO: don't use std::string?
  std::string data_;

  // E.g., example.jpg
  // TODO: Unicode
  std::string file_name_;

  // E.g., image/jpeg
  std::string mime_type_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_FILE_H_
