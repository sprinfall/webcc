#include "webcc/common.h"

#include <codecvt>

#include "boost/algorithm/string.hpp"
#include "boost/filesystem/fstream.hpp"

#include "webcc/logger.h"
#include "webcc/utility.h"

namespace bfs = boost::filesystem;

namespace webcc {

// -----------------------------------------------------------------------------

namespace misc_strings {

// Literal strings can't be used because they have an extra '\0'.

const char HEADER_SEPARATOR[] = { ':', ' ' };
const char CRLF[] = { '\r', '\n' };

}  // misc_strings

// -----------------------------------------------------------------------------

// Read entire file into string.
static bool ReadFile(const Path& path, std::string* output) {
  // Flag "ate": seek to the end of stream immediately after open.
  bfs::ifstream stream{ path, std::ios::binary | std::ios::ate };
  if (stream.fail()) {
    return false;
  }

  auto size = stream.tellg();
  output->resize(static_cast<std::size_t>(size), '\0');
  stream.seekg(std::ios::beg);
  stream.read(&(*output)[0], size);
  if (stream.fail()) {
    return false;
  }
  return true;
}

// -----------------------------------------------------------------------------

void Headers::Set(const std::string& key, const std::string& value) {
  auto it = Find(key);
  if (it != headers_.end()) {
    it->second = value;
  } else {
    headers_.push_back({ key, value });
  }
}

void Headers::Set(std::string&& key, std::string&& value) {
  auto it = Find(key);
  if (it != headers_.end()) {
    it->second = std::move(value);
  } else {
    headers_.push_back({ std::move(key), std::move(value) });
  }
}

bool Headers::Has(const std::string& key) const {
  return const_cast<Headers*>(this)->Find(key) != headers_.end();
}

const std::string& Headers::Get(const std::string& key,
                                    bool* existed) const {
  auto it = const_cast<Headers*>(this)->Find(key);

  if (existed != nullptr) {
    *existed = (it != headers_.end());
  }

  if (it != headers_.end()) {
    return it->second;
  }

  static const std::string s_no_value;
  return s_no_value;
}

std::vector<Header>::iterator Headers::Find(const std::string& key) {
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
  if (!utility::SplitKV(str, '=', &key, value)) {
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

  std::size_t pos = str.find(';');
  if (pos == std::string::npos) {
    media_type_ = str;
  } else {
    media_type_ = str.substr(0, pos);
    other = str.substr(pos + 1);
  }

  boost::trim(media_type_);
  boost::trim(other);

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
    if (!utility::SplitKV(parts[i], '=', &key, &value)) {
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
                   const std::string& media_type)
    : name_(name), media_type_(media_type) {
  if (!ReadFile(path, &data_)) {
    throw Error{ Error::kFileError, "Cannot read the file." };
  }

  // Determine file name from file path.
  // TODO: encoding
  file_name_ = path.filename().string(std::codecvt_utf8<wchar_t>());

  // Determine media type from file extension.
  if (media_type_.empty()) {
    std::string extension = path.extension().string();
    media_type_ = media_types::FromExtension(extension, false);
  }
}

FormPart::FormPart(const std::string& name, std::string&& data,
                   const std::string& media_type)
    : name_(name), data_(std::move(data)), media_type_(media_type) {
}

void FormPart::Prepare(Payload* payload) {
  // The payload buffers don't own the memory.
  // It depends on some existing variables/objects to keep the memory.
  // That's why we need |headers_|.
  if (headers_.empty()) {
    SetHeaders();
  }

  using boost::asio::buffer;

  for (const Header& h : headers_.data()) {
    payload->push_back(buffer(h.first));
    payload->push_back(buffer(misc_strings::HEADER_SEPARATOR));
    payload->push_back(buffer(h.second));
    payload->push_back(buffer(misc_strings::CRLF));
  }

  payload->push_back(buffer(misc_strings::CRLF));

  if (!data_.empty()) {
    payload->push_back(buffer(data_));
  }

  payload->push_back(buffer(misc_strings::CRLF));
}

void FormPart::SetHeaders() {
  // Header: Content-Disposition

  std::string content_disposition = "form-data";
  if (!name_.empty()) {
    content_disposition.append("; name=\"" + name_ + "\"");
  }
  if (!file_name_.empty()) {
    content_disposition.append("; filename=\"" + file_name_ + "\"");
  }
  headers_.Set(headers::kContentDisposition, content_disposition);

  // Header: Content-Type

  if (!media_type_.empty()) {
    headers_.Set(headers::kContentType, media_type_);
  }
}

}  // namespace webcc
