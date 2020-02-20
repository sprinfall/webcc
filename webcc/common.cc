#include "webcc/common.h"

#include <codecvt>

#include "webcc/logger.h"
#include "webcc/string.h"
#include "webcc/utility.h"

namespace webcc {

// -----------------------------------------------------------------------------

bool Headers::Set(const std::string& key, const std::string& value) {
  if (value.empty()) {
    return false;
  }

  auto it = Find(key);
  if (it != headers_.end()) {
    it->second = value;
  } else {
    headers_.push_back({ key, value });
  }

  return true;
}

bool Headers::Set(std::string&& key, std::string&& value) {
  if (value.empty()) {
    return false;
  }

  auto it = Find(key);
  if (it != headers_.end()) {
    it->second = std::move(value);
  } else {
    headers_.push_back({ std::move(key), std::move(value) });
  }

  return true;
}

bool Headers::Has(const std::string& key) const {
  return const_cast<Headers*>(this)->Find(key) != headers_.end();
}

const std::string& Headers::Get(const std::string& key, bool* existed) const {
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
    if (iequals(it->first, key)) {
      break;
    }
  }
  return it;
}

// -----------------------------------------------------------------------------

static bool ParseValue(const std::string& str, const std::string& expected_key,
                       std::string* value) {
  std::string key;
  if (!split_kv(key, *value, str, '=')) {
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
  Reset();
  Init(str);
}

void ContentType::Reset() {
  media_type_.clear();
  additional_.clear();
  multipart_ = false;
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

  trim(media_type_);
  trim(other);

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

static inline void Unquote(std::string& str) {
  trim(str, "\"");
}

bool ContentDisposition::Init(const std::string& str) {
  std::vector<std::string> parts;
  split(parts, str, ';');

  if (parts.empty()) {
    return false;
  }

  if (parts[0] != "form-data") {
    return false;
  }

  std::string key;
  std::string value;
  for (std::size_t i = 1; i < parts.size(); ++i) {
    if (!split_kv(key, value, parts[i], '=')) {
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

FormPartPtr FormPart::New(const std::string& name, std::string&& data,
                          const std::string& media_type) {
  auto form_part = std::make_shared<FormPart>();

  form_part->name_ = name;
  form_part->data_ = std::move(data);
  form_part->media_type_ = media_type;

  return form_part;
}

FormPartPtr FormPart::NewFile(const std::string& name,
                              const std::filesystem::path& path,
                              const std::string& media_type) {
  auto form_part = std::make_shared<FormPart>();

  form_part->name_ = name;
  form_part->path_ = path;
  form_part->media_type_ = media_type;

  // Determine file name from file path.
  // TODO: encoding
  form_part->file_name_ = path.filename().string();

  // Determine media type from file extension.
  // TODO: Default to "application/text"?
  if (form_part->media_type_.empty()) {
    auto ext = path.extension().string();
    form_part->media_type_ = media_types::FromExtension(ext);
  }

  return form_part;
}

void FormPart::Prepare(Payload* payload) {
  using asio::buffer;

  if (data_.empty() && !path_.empty()) {
    if (!utility::ReadFile(path_, &data_)) {
      throw Error{ Error::kFileError, "Cannot read the file" };
    }
  }

  // NOTE:
  // The payload buffers don't own the memory.
  // It depends on some existing variables/objects to keep the memory.
  // That's why we need save headers to member variable.

  if (headers_.empty()) {
    SetHeaders();
  }

  for (const Header& h : headers_.data()) {
    payload->push_back(buffer(h.first));
    payload->push_back(buffer(literal_buffers::HEADER_SEPARATOR));
    payload->push_back(buffer(h.second));
    payload->push_back(buffer(literal_buffers::CRLF));
  }

  payload->push_back(buffer(literal_buffers::CRLF));

  if (!data_.empty()) {
    payload->push_back(buffer(data_));
  }

  payload->push_back(buffer(literal_buffers::CRLF));
}

void FormPart::Free() {
  data_.clear();
  data_.shrink_to_fit();
}

std::size_t FormPart::GetSize() {
  std::size_t size = 0;

  if (headers_.empty()) {
    SetHeaders();
  }

  for (const Header& h : headers_.data()) {
    size += h.first.size();
    size += sizeof(literal_buffers::HEADER_SEPARATOR);
    size += h.second.size();
    size += sizeof(literal_buffers::CRLF);
  }
  size += sizeof(literal_buffers::CRLF);

  size += GetDataSize();

  size += sizeof(literal_buffers::CRLF);

  return size;
}

std::size_t FormPart::GetDataSize() {
  if (!data_.empty()) {
    return data_.size();
  }

  auto size = utility::TellSize(path_);
  if (size == kInvalidLength) {
    throw Error{ Error::kFileError, "Cannot read the file" };
  }

  return size;
}

void FormPart::Dump(std::ostream& os, const std::string& prefix) const {
  for (auto& h : headers_.data()) {
    os << prefix << h.first << ": " << h.second << std::endl;
  }

  os << prefix << std::endl;

  if (!path_.empty()) {
    os << prefix << "<file: " << path_.string() << ">" << std::endl;
  } else {
    utility::DumpByLine(data_, os, prefix);
  }
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
