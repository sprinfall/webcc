#ifndef WEBCC_REQUEST_PARSER_H_
#define WEBCC_REQUEST_PARSER_H_

#include <functional>
#include <string>

#include "webcc/message_parser.h"

namespace webcc {

// Parameters: http_method, url_path, [out]stream
using ViewMatcher =
    std::function<bool(const std::string&, const std::string&, bool*)>;

class Request;

class RequestParser : public MessageParser {
public:
  RequestParser() = default;

  ~RequestParser() override = default;

  void Init(Request* request, ViewMatcher view_matcher);

private:
  // Override to match the URL against views and check if the matched view asks
  // for data streaming.
  bool OnHeadersEnd() override;

  bool ParseStartLine(const std::string& line) override;

  // Override to handle multipart form data which is request only.
  bool ParseContent(const char* data, std::size_t length) override;

  // Multipart specific parsing helpers.

  bool ParseMultipartContent(const char* data, std::size_t length);
  bool ParsePartHeaders(bool* need_more_data);

  bool GetNextBoundaryLine(std::size_t* b_off, std::size_t* b_count,
                           bool* ended);

  // Check if the str.substr(off, count) is a boundary.
  bool IsBoundary(const std::string& str, std::size_t off,
                  std::size_t count, bool* end = nullptr) const;

private:
  // The result request message.
  Request* request_ = nullptr;

  // A function for matching view once the headers of a request has been
  // received. The parsing will stop and fail if no view can be matched.
  ViewMatcher view_matcher_;

  // Form data parsing steps.
  enum class Step {
    kStart,
    kBoundaryParsed,
    kHeadersParsed,
    kEnded,
  };
  Step step_ = Step::kStart;

  // The current form part being parsed.
  FormPartPtr part_;

  // All form parts parsed.
  std::vector<FormPartPtr> form_parts_;
};

}  // namespace webcc

#endif  // WEBCC_REQUEST_PARSER_H_
