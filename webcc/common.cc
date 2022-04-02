#include "webcc/common.h"

#include <codecvt>

#include "boost/algorithm/string.hpp"

#include "webcc/logger.h"
#include "webcc/string.h"
#include "webcc/utility.h"

namespace webcc {

// -----------------------------------------------------------------------------

bool Headers::Set(string_view key, string_view value) {
  if (value.empty()) {
    return false;
  }

  auto it = Find(key);
  if (it != headers_.end()) {
    it->second = ToString(value);
  } else {
    headers_.push_back({ ToString(key), ToString(value) });
  }

  return true;
}

bool Headers::Has(string_view key) const {
  return const_cast<Headers*>(this)->Find(key) != headers_.end();
}

const std::string& Headers::Get(string_view key, bool* existed) const {
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

std::vector<Header>::iterator Headers::Find(string_view key) {
  auto it = headers_.begin();
  for (; it != headers_.end(); ++it) {
    if (boost::iequals(it->first, key)) {
      break;
    }
  }
  return it;
}

// -----------------------------------------------------------------------------

static bool ParseValue(const std::string& str, const char* expected_key,
                       string_view* value) {
  string_view key;
  if (!SplitKV(str, '=', true, &key, value)) {
    return false;
  }
  if (key != expected_key) {
    return false;
  }
  return !value->empty();
}

ContentType::ContentType(string_view str) {
  Init(str);
}

void ContentType::Parse(string_view str) {
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

void ContentType::Init(string_view str) {
  std::string other;

  std::size_t pos = str.find(';');
  if (pos == str.npos) {
    media_type_ = ToString(str);
  } else {
    media_type_ = ToString(str.substr(0, pos));
    other = ToString(str.substr(pos + 1));
  }

  boost::trim(media_type_);
  boost::trim(other);

  if (media_type_ == "multipart/form-data") {
    multipart_ = true;
    string_view boundary;
    if (ParseValue(other, "boundary", &boundary)) {
      additional_ = ToString(boundary);
      LOG_INFO("Content-type multipart boundary: %s", additional_.c_str());
    } else {
      LOG_ERRO("Invalid 'multipart/form-data' content-type (no boundary)");
    }
  } else {
    string_view charset;
    if (ParseValue(other, "charset", &charset)) {
      additional_ = ToString(charset);
      LOG_INFO("Content-type charset: %s", additional_.c_str());
    }
  }
}

// -----------------------------------------------------------------------------

// TODO: Use string_view
static inline void Unquote(std::string& str) {
  boost::trim_if(str, boost::is_any_of("\""));
}

bool ContentDisposition::Init(string_view str) {
  std::vector<string_view> parts;
  Split(str, ';', false, &parts);

  if (parts.empty()) {
    return false;
  }

  if (parts[0] != "form-data") {
    return false;
  }

  string_view key;
  string_view value;
  for (std::size_t i = 1; i < parts.size(); ++i) {
    if (!SplitKV(parts[i], '=', true, &key, &value)) {
      return false;
    }

    if (key == "name") {
      name_ = ToString(value);
      Unquote(name_);
    } else if (key == "filename") {
      file_name_ = ToString(value);
      Unquote(file_name_);
    }
  }

  return true;
}

// -----------------------------------------------------------------------------

FormPartPtr FormPart::New(string_view name, std::string&& data,
                          string_view media_type) {
  auto form_part = std::make_shared<FormPart>();

  form_part->name_ = ToString(name);
  form_part->data_ = std::move(data);
  form_part->media_type_ = ToString(media_type);

  return form_part;
}

FormPartPtr FormPart::NewFile(string_view name, const sfs::path& path,
                              string_view media_type) {
  auto form_part = std::make_shared<FormPart>();

  form_part->name_ = ToString(name);
  form_part->path_ = path;
  form_part->media_type_ = ToString(media_type);

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
  using boost::asio::buffer;

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

void FormPart::Dump(std::ostream& os, string_view prefix) const {
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
