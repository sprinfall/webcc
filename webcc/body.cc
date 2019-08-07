#include "webcc/body.h"

#include "boost/algorithm/string.hpp"
#include "boost/core/ignore_unused.hpp"

#include "webcc/logger.h"
#include "webcc/utility.h"

#if WEBCC_ENABLE_GZIP
#include "webcc/gzip.h"
#endif

namespace bfs = boost::filesystem;

namespace webcc {

// -----------------------------------------------------------------------------

#if WEBCC_ENABLE_GZIP

bool StringBody::Compress() {
  if (compressed_) {
    return true;  // Already compressed.
  }

  if (data_.size() <= kGzipThreshold) {
    return false;
  }

  std::string compressed;
  if (gzip::Compress(data_, &compressed)) {
    data_ = std::move(compressed);
    compressed_ = true;
    return true;
  }

  LOG_WARN("Failed to compress the body data!");
  return false;
}

bool StringBody::Decompress() {
  if (!compressed_) {
    return true;  // Already decompressed.
  }

  std::string decompressed;
  if (gzip::Decompress(data_, &decompressed)) {
    data_ = std::move(decompressed);
    compressed_ = false;
    return true;
  }

  LOG_WARN("Failed to decompress the body data!");
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
    return { boost::asio::buffer(data_) };
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

// -----------------------------------------------------------------------------

FileBody::FileBody(const Path& path, std::size_t chunk_size)
    : path_(path), chunk_size_(chunk_size) {
  size_ = utility::TellSize(path_);
  if (size_ == kInvalidLength) {
    throw Error{ Error::kFileError, "Cannot read the file" };
  }
}

void FileBody::InitPayload() {
  assert(chunk_size_ > 0);
  chunk_.resize(chunk_size_);

  if (stream_.is_open()) {
    stream_.close();
  }

  stream_.open(path_, std::ios::binary);

  if (stream_.fail()) {
    throw Error{ Error::kFileError, "Cannot read the file" };
  }
}

Payload FileBody::NextPayload(bool free_previous) {
  boost::ignore_unused(free_previous);

  if (stream_.read(&chunk_[0], chunk_.size()).gcount() > 0) {
    return {
      boost::asio::buffer(chunk_.data(), (std::size_t)stream_.gcount())
    };
  }
  return {};
}

void FileBody::Dump(std::ostream& os, const std::string& prefix) const {
  os << prefix << "<file: " << path_.string() << ">" << std::endl;
}

}  // namespace webcc
