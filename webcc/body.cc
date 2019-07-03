#include "webcc/body.h"

#include "boost/algorithm/string.hpp"

#include "webcc/logger.h"
#include "webcc/utility.h"

#if WEBCC_ENABLE_GZIP
#include "webcc/gzip.h"
#endif

namespace webcc {

// -----------------------------------------------------------------------------

namespace misc_strings {

// Literal strings can't be used because they have an extra '\0'.

const char CRLF[] = { '\r', '\n' };
const char DOUBLE_DASHES[] = { '-', '-' };

}  // misc_strings

// -----------------------------------------------------------------------------

#if WEBCC_ENABLE_GZIP
bool StringBody::Compress() {
  if (data_.size() <= kGzipThreshold) {
    return false;
  }

  std::string compressed;
  if (gzip::Compress(data_, &compressed)) {
    data_ = std::move(compressed);
    return true;
  }

  LOG_WARN("Failed to compress the body data!");
  return false;
}
#endif  // WEBCC_ENABLE_GZIP

void StringBody::InitPayload() {
  index_ = 0;
}

Payload StringBody::NextPayload() {
  if (index_ == 0) {
    index_ = 1;
    return Payload{ boost::asio::buffer(data_) };
  }
  return {};
}

// NOTE:
// - The data will be truncated if it's too large to display.
// - Binary content will not be dumped (TODO).
void StringBody::Dump(std::ostream& os, const std::string& prefix) const {
  if (data_.empty()) {
    return;
  }

  // Split by EOL to achieve more readability.
  std::vector<std::string> lines;
  boost::split(lines, data_, boost::is_any_of("\n"));

  std::size_t size = 0;

  for (const std::string& line : lines) {
    os << prefix;

    if (line.size() + size > kMaxDumpSize) {
      os.write(line.c_str(), kMaxDumpSize - size);
      os << "..." << std::endl;
      break;
    } else {
      os << line << std::endl;
      size += line.size();
    }
  }
}

// -----------------------------------------------------------------------------

FormBody::FormBody(const std::vector<FormPartPtr>& parts,
                   const std::string& boundary)
    : parts_(parts), boundary_(boundary) {
}

std::size_t FormBody::GetSize() const {
  std::size_t size = 0;

  for (auto& part : parts_) {
    size += boundary_.size() + 4;  // 4: -- and CRLF
    size += part->GetSize();
  }

  size += boundary_.size() + 6;

  return size;
}

void FormBody::Dump(std::ostream& os, const std::string& prefix) const {
  // TODO
}

void FormBody::InitPayload() {
  index_ = 0;
}

// TODO: Clear previous payload memory.
Payload FormBody::NextPayload() {
  Payload payload;

  if (index_ < parts_.size()) {
    auto& part = parts_[index_];
    AddBoundary(&payload);
    part->Prepare(&payload);

    if (index_ + 1 == parts_.size()) {
      AddBoundaryEnd(&payload);
    }
  }

  return payload;
}

void FormBody::AddBoundary(Payload* payload) {
  using boost::asio::buffer;
  payload->push_back(buffer(misc_strings::DOUBLE_DASHES));
  payload->push_back(buffer(boundary_));
  payload->push_back(buffer(misc_strings::CRLF));
}

void FormBody::AddBoundaryEnd(Payload* payload) {
  using boost::asio::buffer;
  payload->push_back(buffer(misc_strings::DOUBLE_DASHES));
  payload->push_back(buffer(boundary_));
  payload->push_back(buffer(misc_strings::DOUBLE_DASHES));
  payload->push_back(buffer(misc_strings::CRLF));
}

}  // namespace webcc
