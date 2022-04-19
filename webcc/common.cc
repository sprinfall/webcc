#include "webcc/common.h"

#include <codecvt>

#include "boost/algorithm/string.hpp"

#include "webcc/logger.h"
#include "webcc/string.h"
#include "webcc/utility.h"

namespace webcc {

// -----------------------------------------------------------------------------

bool Headers::Set(std::string_view key, std::string_view value) {
  if (key.empty() || value.empty()) {
    return false;
  }

  auto iter = Find(key);
  if (iter != headers_.end()) {
    iter->second = value;
  } else {
    headers_.emplace_back(key, value);
  }

  return true;
}

std::vector<Header>::iterator Headers::Find(std::string_view key) {
  auto iter = headers_.begin();
  for (; iter != headers_.end(); ++iter) {
    if (boost::iequals(iter->first, key)) {
      break;
    }
  }
  return iter;
}

// -----------------------------------------------------------------------------

static bool ParseValue(std::string_view str, const char* expected_key,
                       std::string_view* value) {
  std::string_view key;
  if (!SplitKV(str, '=', true, &key, value)) {
    return false;
  }
  if (key != expected_key) {
    return false;
  }
  return !value->empty();
}

ContentType::ContentType(std::string_view str) {
  Init(str);
}

void ContentType::Parse(std::string_view str) {
  Clear();
  Init(str);
}

void ContentType::Clear() {
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

void ContentType::Init(std::string_view str) {
  std::string_view other;

  std::size_t pos = str.find(';');
  if (pos == std::string_view::npos) {
    media_type_ = str;
  } else {
    media_type_ = str.substr(0, pos);
    other = str.substr(pos + 1);
  }

  boost::trim(media_type_);
  Trim(other);

  if (media_type_ == "multipart/form-data") {
    multipart_ = true;
    std::string_view boundary;
    if (ParseValue(other, "boundary", &boundary)) {
      additional_ = boundary;
      LOG_INFO("Content-type multipart boundary: %s", additional_.c_str());
    } else {
      LOG_ERRO("Invalid 'multipart/form-data' content-type (no boundary)");
    }
  } else {
    std::string_view charset;
    if (ParseValue(other, "charset", &charset)) {
      additional_ = charset;
      LOG_INFO("Content-type charset: %s", additional_.c_str());
    }
  }
}

// -----------------------------------------------------------------------------

bool ContentDisposition::Init(std::string_view str) {
  std::vector<std::string_view> parts;
  Split(str, ';', false, &parts);

  if (parts.empty()) {
    return false;
  }

  if (parts[0] != "form-data") {
    return false;
  }

  std::string_view key;
  std::string_view value;
  for (std::size_t i = 1; i < parts.size(); ++i) {
    if (!SplitKV(parts[i], '=', true, &key, &value)) {
      return false;
    }

    if (key == "name") {
      name_ = Unquote(value);
    } else if (key == "filename") {
      file_name_ = Unquote(value);
    }
  }

  return true;
}

// -----------------------------------------------------------------------------

FormPartPtr FormPart::New(std::string_view name, std::string&& data,
                          std::string_view media_type) {
  auto form_part = std::make_shared<FormPart>();

  form_part->name_ = name;
  form_part->data_ = std::move(data);
  form_part->media_type_ = media_type;

  return form_part;
}

FormPartPtr FormPart::NewFile(std::string_view name, const sfs::path& path,
                              std::string_view media_type) {
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

  constexpr std::size_t CRLF_SIZE = sizeof(literal_buffers::CRLF);

  for (const Header& h : headers_.data()) {
    size += h.first.size();
    size += sizeof(literal_buffers::HEADER_SEPARATOR);
    size += h.second.size();
    size += CRLF_SIZE;
  }

  size += CRLF_SIZE;
  size += GetDataSize();
  size += CRLF_SIZE;

  return size;
}

std::size_t FormPart::GetDataSize() {
  if (!data_.empty()) {
    return data_.size();
  }

  std::size_t size = utility::TellSize(path_);
  if (size == kInvalidLength) {
    throw Error{ Error::kFileError, "Cannot read the file" };
  }

  return size;
}

void FormPart::Dump(std::ostream& os, std::string_view prefix) const {
  for (const Header& h : headers_.data()) {
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
