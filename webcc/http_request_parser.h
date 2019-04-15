#ifndef WEBCC_HTTP_REQUEST_PARSER_H_
#define WEBCC_HTTP_REQUEST_PARSER_H_

#include <string>

#include "webcc/http_parser.h"

namespace webcc {

class HttpRequest;

class HttpRequestParser : public HttpParser {
public:
  explicit HttpRequestParser(HttpRequest* request = nullptr);

  ~HttpRequestParser() override = default;

  void Init(HttpRequest* request);

private:
  bool ParseStartLine(const std::string& line) final;

  // Override to handle multipart form data which is request only.
  bool ParseContent(const char* data, std::size_t length) final;

  // Multipart specific parsing helpers.

  bool ParseMultipartContent(const char* data, std::size_t length);
  bool ParsePartHeaders(bool* need_more_data);
  bool GetNextBoundaryLine(std::size_t* b_off, std::size_t* b_count,
                           bool* ended);
  bool IsBoundary(const std::string& line) const;
  bool IsBoundaryEnd(const std::string& line) const;

private:
  HttpRequest* request_;

  enum Step {
    kStart,
    kBoundaryParsed,
    kHeadersParsed,
    kEnded,
  };
  Step step_ = kStart;

  FormPart part_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_REQUEST_PARSER_H_
