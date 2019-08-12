#ifndef WEBCC_REQUEST_PARSER_H_
#define WEBCC_REQUEST_PARSER_H_

#include <string>

#include "webcc/parser.h"

namespace webcc {

class Request;

class RequestParser : public Parser {
public:
  RequestParser();

  ~RequestParser() override = default;

  void Init(Request* request);

private:
  void CreateBodyHandler() override;

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
  Request* request_;

  // Form data parsing step.
  enum Step {
    kStart,
    kBoundaryParsed,
    kHeadersParsed,
    kEnded,
  };
  Step step_ = kStart;

  // The current form part being parsed.
  FormPartPtr part_;

  // All form parts parsed.
  std::vector<FormPartPtr> form_parts_;
};

}  // namespace webcc

#endif  // WEBCC_REQUEST_PARSER_H_
