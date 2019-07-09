#include "webcc/body.h"

#include "boost/algorithm/string.hpp"
#include "boost/core/ignore_unused.hpp"

#include "webcc/logger.h"
#include "webcc/utility.h"

#if WEBCC_ENABLE_GZIP
#include "webcc/gzip.h"
#endif

namespace webcc {

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

Payload StringBody::NextPayload(bool free_previous) {
  boost::ignore_unused(free_previous);

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
  if (!data_.empty()) {
    utility::DumpByLine(data_, os, prefix);
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
  for (auto& part : parts_) {
    os << prefix << "--" << boundary_ << std::endl;
    part->Dump(os, prefix);
  }
  os << prefix << "--" << boundary_ << "--" << std::endl;
}

void FormBody::InitPayload() {
  index_ = 0;
}

Payload FormBody::NextPayload(bool free_previous) {
  Payload payload;
  
  // Free previous payload.
  if (free_previous) {
    if (index_ > 0) {
      Free(index_ - 1);
    }
  }

  if (index_ < parts_.size()) {
    AddBoundary(&payload);
    parts_[index_]->Prepare(&payload);

    if (index_ + 1 == parts_.size()) {
      AddBoundaryEnd(&payload);
    }
  }

  ++index_;

  return payload;
}

void FormBody::AddBoundary(Payload* payload) {
  using boost::asio::buffer;

  payload->push_back(buffer(literal_buffers::DOUBLE_DASHES));
  payload->push_back(buffer(boundary_));
  payload->push_back(buffer(literal_buffers::CRLF));
}

void FormBody::AddBoundaryEnd(Payload* payload) {
  using boost::asio::buffer;

  payload->push_back(buffer(literal_buffers::DOUBLE_DASHES));
  payload->push_back(buffer(boundary_));
  payload->push_back(buffer(literal_buffers::DOUBLE_DASHES));
  payload->push_back(buffer(literal_buffers::CRLF));
}

void FormBody::Free(std::size_t index) {
  if (index < parts_.size()) {
    parts_[index]->Free();
  }
}

}  // namespace webcc
