#include "webcc/request_parser.h"

#include <vector>

#include "boost/algorithm/string.hpp"

#include "webcc/internal/globals.h"
#include "webcc/logger.h"
#include "webcc/request.h"
#include "webcc/string.h"
#include "webcc/utility.h"

namespace webcc {

void RequestParser::Init(Request* request, ViewMatcher view_matcher) {
  assert(view_matcher != nullptr);

  MessageParser::Init(request);

  request_ = request;
  view_matcher_ = view_matcher;

  step_ = Step::kStart;
  part_.reset();
  form_parts_.clear();
}

bool RequestParser::OnHeadersEnd() {
  // Decode the URL path before match.
  std::string url_path = Url::DecodeUnsafe(request_->url().path());
  if (view_matcher_(request_->method(), url_path, &stream_)) {
    if (stream_) {
      LOG_INFO("The URL path matches a view which askes for data streaming");
    }
  }  // else: Do nothing!

  // Always return true, even if no view matches the URL path, so that the
  // request could be fully received.
  return true;
}

bool RequestParser::ParseStartLine(const std::string& line) {
  std::vector<std::string_view> parts;
  Split(line, ' ', true, &parts);

  if (parts.size() != 3) {
    return false;
  }

  request_->set_method(parts[0]);
  request_->set_url(Url{ parts[1] });

  // HTTP version is ignored.

  return true;
}

bool RequestParser::ParseContent(const char* data, std::size_t length) {
  if (content_type_.multipart()) {
    return ParseMultipartContent(data, length);
  } else {
    return MessageParser::ParseContent(data, length);
  }
}

bool RequestParser::ParseMultipartContent(const char* data,
                                          std::size_t length) {
  pending_data_.append(data, length);

  if (!content_length_parsed_ || content_length_ == kInvalidSize) {
    // Invalid Content-Length header (syntax error).
    return false;
  }

  while (true) {
    if (pending_data_.empty()) {
      // Wait data from next read.
      break;
    }

    if (step_ == Step::kStart) {
      std::string line;
      if (!GetNextLine(0, &line, true)) {
        break;  // Not enough data
      }
      if (!IsBoundary(line, 0, line.size())) {
        LOG_ERRO("Invalid boundary: %s", line.c_str());
        return false;
      }
      LOG_INFO("Boundary line: %s", line.c_str());
      // Go to next step.
      step_ = Step::kBoundaryParsed;
      continue;
    }

    if (step_ == Step::kBoundaryParsed) {
      if (part_ == nullptr) {
        part_.reset(new FormPart{});
      }
      bool need_more_data = false;
      if (ParsePartHeaders(&need_more_data)) {
        // Go to next step.
        step_ = Step::kHeadersParsed;
        LOG_INFO("Part headers just ended");
        continue;
      } else {
        if (need_more_data) {
          // Need more data from next read.
          break;
        } else {
          return false;
        }
      }
    }

    if (step_ == Step::kHeadersParsed) {
      // TODO: Remember last CRLF position.

      std::size_t off = 0;
      std::size_t count = 0;
      bool ended = false;
      if (!GetNextBoundaryLine(&off, &count, &ended)) {
        break;
      }

      // Next boundary found.
      LOG_INFO("Next boundary found, off=%u", off);

      // This part has ended.
      if (off >= 2) {
        // -2 for excluding the CRLF after the data.
        part_->AppendData(pending_data_.data(), off - 2);

        // Erase the data of this part and the next boundary.
        // +2 for including the CRLF after the boundary.
        pending_data_.erase(0, off + count + 2);
      } else {
        LOG_ERRO("Invalid part data, off=%u", off);
        return false;
      }

      // Save this part
      form_parts_.push_back(part_);

      // Reset for next part.
      part_.reset();

      if (ended) {
        // Go to the end step.
        step_ = Step::kEnded;
        break;
      } else {
        // Go to next step.
        step_ = Step::kBoundaryParsed;
        continue;
      }
    }
  }

  if (step_ == Step::kEnded) {
    LOG_INFO("Multipart data has ended");

    // Create a body and set to the request.

    auto body = std::make_shared<FormBody>(form_parts_,
                                           content_type_.boundary());

    request_->SetBody(body, false);  // TODO: set_length?

    Finish();
  }

  return true;
}

bool RequestParser::ParsePartHeaders(bool* need_more_data) {
  std::size_t off = 0;

  while (true) {
    std::string line;
    if (!GetNextLine(off, &line, false)) {
      // Need more data from next read.
      *need_more_data = true;
      return false;
    }

    off = off + line.size() + 2;  // +2 for CRLF

    if (line.empty()) {
      // Headers finished.
      break;
    }

    Header header;
    if (!SplitKV(line, ':', true, &header.first, &header.second)) {
      LOG_ERRO("Invalid part header line: %s", line.c_str());
      return false;
    }

    LOG_INFO("Part header (%s: %s)", header.first.c_str(),
             header.second.c_str());

    // Parse Content-Disposition.
    if (boost::iequals(header.first, headers::kContentDisposition)) {
      ContentDisposition content_disposition{ header.second };
      if (!content_disposition.valid()) {
        LOG_ERRO("Invalid content-disposition header: %s",
                 header.second.c_str());
        return false;
      }
      part_->set_name(content_disposition.name());
      part_->set_file_name(content_disposition.file_name());
      LOG_INFO("Content-Disposition (name=%s; filename=%s)",
               part_->name().c_str(), part_->file_name().c_str());
    }

    // TODO: Parse other headers.
  }

  // Remove the data which has just been parsed.
  pending_data_.erase(0, off);

  return true;
}

bool RequestParser::GetNextBoundaryLine(std::size_t* b_off,
                                        std::size_t* b_count, bool* ended) {
  std::size_t off = 0;

  while (true) {
    std::size_t pos = pending_data_.find(internal::kCRLF, off);
    if (pos == std::string::npos) {
      break;
    }

    std::size_t count = pos - off;
    if (count == 0) {
      off = pos + 2;
      continue;  // Empty line
    }

    if (IsBoundary(pending_data_, off, count, ended)) {
      *b_off = off;
      *b_count = count;
      return true;
    }

    off = pos + 2;
  }

  return false;
}

bool RequestParser::IsBoundary(const std::string& str, std::size_t off,
                               std::size_t count, bool* end) const {
  const std::string& boundary = content_type_.boundary();

  if (count != boundary.size() + 2 && count != boundary.size() + 4) {
    return false;
  }

  if (str[off] != '-' || str[off + 1] != '-') {
    return false;
  }

  if (count == boundary.size() + 4) {
    if (str[off + count - 1] != '-' || str[off + count - 2] != '-') {
      return false;
    }
    if (end != nullptr) {
      *end = true;
    }
  }

  return std::strncmp(boundary.c_str(), &str[off + 2], boundary.size()) == 0;
}

}  // namespace webcc
