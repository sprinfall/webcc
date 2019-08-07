#ifndef WEBCC_BODY_H_
#define WEBCC_BODY_H_

#include <memory>
#include <string>
#include <utility>

#include "boost/filesystem/fstream.hpp"

#include "webcc/common.h"

namespace webcc {

// -----------------------------------------------------------------------------

class Body {
public:
  Body() = default;
  virtual ~Body() = default;

  // Get the size in bytes of the body.
  virtual std::size_t GetSize() const {
    return 0;
  }

  bool IsEmpty() const {
    return GetSize() == 0;
  }

#if WEBCC_ENABLE_GZIP

  // Compress the data with Gzip.
  // If data size <= threshold (1400 bytes), no compression will be taken
  // and false will be simply returned.
  virtual bool Compress() {
    return false;
  }

  // Decompress the data.
  virtual bool Decompress() {
    return false;
  }

#endif  // WEBCC_ENABLE_GZIP

  // Initialize the payload for iteration.
  // Usage:
  //   InitPayload();
  //   for (auto p = NextPayload(); !p.empty(); p = NextPayload()) {
  //   }
  virtual void InitPayload() {
  }

  // Get the next payload.
  // An empty payload returned indicates the end.
  virtual Payload NextPayload(bool free_previous = false) {
    return {};
  }

  // Dump to output stream for logging purpose.
  virtual void Dump(std::ostream& os, const std::string& prefix) const {
  }
};

using BodyPtr = std::shared_ptr<Body>;

// -----------------------------------------------------------------------------

class StringBody : public Body {
public:
  explicit StringBody(const std::string& data, bool compressed)
      : data_(data), compressed_(compressed) {
  }

  explicit StringBody(std::string&& data, bool compressed)
      : data_(std::move(data)), compressed_(compressed) {
  }

  std::size_t GetSize() const override {
    return data_.size();
  }

  const std::string& data() const {
    return data_;
  }

#if WEBCC_ENABLE_GZIP

  bool Compress() override;

  bool Decompress() override;

#endif  // WEBCC_ENABLE_GZIP

  void InitPayload() override;

  Payload NextPayload(bool free_previous = false) override;

  void Dump(std::ostream& os, const std::string& prefix) const override;

private:
  std::string data_;

  // Is the data compressed?
  bool compressed_;

  // Index for (not really) iterating the payload.
  std::size_t index_ = 0;
};

// -----------------------------------------------------------------------------

// Multi-part form body for request.
class FormBody : public Body {
public:
  FormBody(const std::vector<FormPartPtr>& parts, const std::string& boundary);

  std::size_t GetSize() const override;

  const std::vector<FormPartPtr>& parts() const {
    return parts_;
  }

  void InitPayload() override;

  Payload NextPayload(bool free_previous = false) override;

  void Dump(std::ostream& os, const std::string& prefix) const override;

private:
  void AddBoundary(Payload* payload);
  void AddBoundaryEnd(Payload* payload);

  void Free(std::size_t index);

private:
  std::vector<FormPartPtr> parts_;
  std::string boundary_;

  // Index for iterating the payload.
  std::size_t index_ = 0;
};

// -----------------------------------------------------------------------------

// File body for server to serve a file without loading the whole of it into
// the memory.
class FileBody : public Body {
public:
  FileBody(const Path& path, std::size_t chunk_size);

  std::size_t GetSize() const override {
    return size_;
  }

  void InitPayload() override;

  Payload NextPayload(bool free_previous = false) override;

  void Dump(std::ostream& os, const std::string& prefix) const override;

private:
  Path path_;
  std::size_t chunk_size_;

  std::size_t size_;  // File size in bytes

  boost::filesystem::ifstream stream_;
  std::string chunk_;
};

}  // namespace webcc

#endif  // WEBCC_BODY_H_
