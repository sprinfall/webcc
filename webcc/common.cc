#include "webcc/common.h"

#include "boost/algorithm/string.hpp"
#include "boost/filesystem/fstream.hpp"

#include "webcc/logger.h"

namespace bfs = boost::filesystem;

namespace webcc {

// -----------------------------------------------------------------------------

namespace misc_strings {

const char HEADER_SEPARATOR[] = { ':', ' ' };
const char CRLF[] = { '\r', '\n' };

}  // misc_strings

// -----------------------------------------------------------------------------

bool Split2(const std::string& str, char token, std::string* part1,
            std::string* part2) {
  std::size_t pos = str.find(token);
  if (pos == std::string::npos) {
    return false;
  }

  *part1 = str.substr(0, pos);
  *part2 = str.substr(pos + 1);

  boost::trim(*part1);
  boost::trim(*part2);

  return true;
}

bool ReadFile(const Path& path, std::string* output) {
  bfs::ifstream ifs{ path, std::ios::binary | std::ios::ate };
  if (!ifs) {
    return false;
  }

  auto size = ifs.tellg();
  output->resize(static_cast<std::size_t>(size), '\0');
  ifs.seekg(0);
  ifs.read(&(*output)[0], size);  // TODO: Error handling

  return true;
}

// -----------------------------------------------------------------------------

void HttpHeaders::Set(const std::string& key, const std::string& value) {
  auto it = Find(key);
  if (it != headers_.end()) {
    it->second = value;
  } else {
    headers_.push_back({ key, value });
  }
}

void HttpHeaders::Set(std::string&& key, std::string&& value) {
  auto it = Find(key);
  if (it != headers_.end()) {
    it->second = std::move(value);
  } else {
    headers_.push_back({ std::move(key), std::move(value) });
  }
}

bool HttpHeaders::Has(const std::string& key) const {
  return const_cast<HttpHeaders*>(this)->Find(key) != headers_.end();
}

const std::string& HttpHeaders::Get(const std::string& key,
                                    bool* existed) const {
  auto it = const_cast<HttpHeaders*>(this)->Find(key);

  if (existed != nullptr) {
    *existed = (it != headers_.end());
  }

  if (it != headers_.end()) {
    return it->second;
  }

  static const std::string s_no_value;
  return s_no_value;
}

std::vector<HttpHeader>::iterator HttpHeaders::Find(const std::string& key) {
  auto it = headers_.begin();
  for (; it != headers_.end(); ++it) {
    if (boost::iequals(it->first, key)) {
      break;
    }
  }
  return it;
}

// -----------------------------------------------------------------------------

static bool ParseValue(const std::string& str, const std::string& expected_key,
                       std::string* value) {
  std::string key;
  if (!Split2(str, '=', &key, value)) {
    return false;
  }

  if (key != expected_key) {
    return false;
  }

  return !value->empty();
}

ContentType::ContentType(const std::string& str) {
  Init(str);
}

void ContentType::Parse(const std::string& str) {
  media_type_.clear();
  additional_.clear();
  multipart_ = false;

  Init(str);
}

bool ContentType::Valid() const {
  if (media_type_.empty()) {
    return false;
  }

  if (multipart_) {
    return !boundary().empty();
  }

  return true;
}

void ContentType::Init(const std::string& str) {
  std::string other;
  Split2(str, ';', &media_type_, &other);

  if (media_type_ == "multipart/form-data") {
    multipart_ = true;
    if (!ParseValue(other, "boundary", &additional_)) {
      LOG_ERRO("Invalid 'multipart/form-data' content-type (no boundary).");
    } else {
      LOG_INFO("Content-type multipart boundary: %s.", additional_.c_str());
    }
  } else {
    if (ParseValue(other, "charset", &additional_)) {
      LOG_INFO("Content-type charset: %s.", additional_.c_str());
    }
  }
}

// -----------------------------------------------------------------------------

static void Unquote(std::string& str) {
  boost::trim_if(str, boost::is_any_of("\""));
}

bool ContentDisposition::Init(const std::string& str) {
  std::vector<std::string> parts;
  boost::split(parts, str, boost::is_any_of(";"));

  if (parts.empty()) {
    return false;
  }

  if (parts[0] != "form-data") {
    return false;
  }

  std::string key;
  std::string value;
  for (std::size_t i = 1; i < parts.size(); ++i) {
    if (!Split2(parts[i], '=', &key, &value)) {
      return false;
    }

    if (key == "name") {
      name_ = value;
      Unquote(name_);
    } else if (key == "filename") {
      file_name_ = value;
      Unquote(file_name_);
    }
  }

  return true;
}

// -----------------------------------------------------------------------------

FormPart::FormPart(const std::string& name, const Path& path,
                   const std::string& mime_type)
    : name_(name), mime_type_(mime_type) {
  if (!ReadFile(path, &data_)) {
    throw Exception(kFileIOError, "Cannot read the file.");
  }

  // Determine file name from file path.
  // TODO: Encoding
  file_name_ = path.filename().string();

  // Determine content type from file extension.
  if (mime_type_.empty()) {
    std::string extension = path.extension().string();
    mime_type_ = http::media_types::FromExtension(extension, false);
  }
}

FormPart::FormPart(const std::string& name, std::string&& data,
                   const std::string& mime_type)
    : name_(name), data_(std::move(data)), mime_type_(mime_type) {
}

void FormPart::Prepare(std::vector<boost::asio::const_buffer>& payload) {
  if (headers_.empty()) {
    std::string value = "form-data";
    if (!name_.empty()) {
      value.append("; name=\"" + name_ + "\"");
    }
    if (!file_name_.empty()) {
      value.append("; filename=\"" + file_name_ + "\"");
    }
    headers_.Set(http::headers::kContentDisposition, value);

    if (!mime_type_.empty()) {
      headers_.Set(http::headers::kContentType, mime_type_);
    }
  }

  using boost::asio::buffer;

  for (const HttpHeader& h : headers_.data()) {
    payload.push_back(buffer(h.first));
    payload.push_back(buffer(misc_strings::HEADER_SEPARATOR));
    payload.push_back(buffer(h.second));
    payload.push_back(buffer(misc_strings::CRLF));
  }

  payload.push_back(buffer(misc_strings::CRLF));

  if (!data_.empty()) {
    payload.push_back(buffer(data_));
  }

  payload.push_back(buffer(misc_strings::CRLF));
}

}  // namespace webcc
