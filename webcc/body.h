#ifndef WEBCC_BODY_H_
#define WEBCC_BODY_H_

#include <memory>
#include <string>
#include <utility>  // for move()

#include "webcc/common.h"

namespace webcc {

// -----------------------------------------------------------------------------

class Body {
public:
  virtual ~Body() = default;

  // Get the size in bytes of the body.
  virtual std::size_t GetSize() const {
    return 0;
  }

  bool IsEmpty() const {
    return GetSize() == 0;
  }

#if WEBCC_ENABLE_GZIP
  // Compress with Gzip.
  // If the body size <= the threshold (1400 bytes), no compression will be done
  // and just return false.
  virtual bool Compress() {
    return false;
  }
#endif  // WEBCC_ENABLE_GZIP

  // Initialize the payload for iteration.
  // Usage:
  //   InitPayload();
  //   for (auto p = NextPayload(); !p.empty(); p = NextPayload()) {
  //   }
  virtual void InitPayload() {}

  // Get the next payload.
  // An empty payload returned indicates the end.
  virtual Payload NextPayload() {
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
  explicit StringBody(const std::string& data) : data_(data) {
  }

  explicit StringBody(std::string&& data) : data_(std::move(data)) {
  }

  std::size_t GetSize() const override {
    return data_.size();
  }

  const std::string& data() const {
    return data_;
  }

#if WEBCC_ENABLE_GZIP
  bool Compress() override;
#endif

  void InitPayload() override;

  Payload NextPayload() override;

  void Dump(std::ostream& os, const std::string& prefix) const override;

private:
  std::string data_;

  // Index for (not really) iterating the payload.
  std::size_t index_ = 0;
};

// -----------------------------------------------------------------------------

// Multi-part form body for request.
class FormBody : public Body {
public:
  FormBody(const std::vector<FormPartPtr>& parts,
           const std::string& boundary);

  std::size_t GetSize() const override;

  const std::vector<FormPartPtr>& parts() const {
    return parts_;
  }

  void InitPayload() override;

  Payload NextPayload() override;

  void Dump(std::ostream& os, const std::string& prefix) const override;

private:
  void AddBoundary(Payload* payload);
  void AddBoundaryEnd(Payload* payload);

private:
  std::vector<FormPartPtr> parts_;
  std::string boundary_;

  // Index for iterating the payload.
  std::size_t index_ = 0;
};

}  // namespace webcc

#endif  // WEBCC_BODY_H_
