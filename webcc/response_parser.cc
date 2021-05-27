#include "webcc/response_parser.h"

#include "boost/algorithm/string.hpp"

#include "webcc/logger.h"
#include "webcc/response.h"

namespace webcc {

// -----------------------------------------------------------------------------

// Split HTTP response status line to three parts.
// Don't use the general split function because the reason part might also
// contain spaces.
static void SplitStatusLine(const std::string& line,
                            std::vector<std::string>* parts) {
  std::size_t off = 0;
  std::size_t pos = 0;

  for (std::size_t i = 0; i < 2; ++i) {
    pos = line.find(' ', off);
    if (pos == std::string::npos) {
      break;
    }

    parts->push_back(line.substr(off, pos - off));
    off = pos + 1;

    // Skip spaces
    while (off < line.size() && line[off] == ' ') {
      ++off;
    }
  }

  if (off < line.size()) {
    parts->push_back(line.substr(off));
  }
}

// -----------------------------------------------------------------------------

void ResponseParser::Init(Response* response, bool stream) {
  Parser::Init(response);

  response_ = response;
  stream_ = stream;
}

bool ResponseParser::ParseStartLine(const std::string& line) {
  std::vector<std::string> parts;
  SplitStatusLine(line, &parts);

  if (parts.size() < 2) {
    LOG_ERRO("Invalid HTTP response status line: %s", line.c_str());
    return false;
  }

  if (!boost::starts_with(parts[0], "HTTP/1.")) {
    LOG_ERRO("Invalid HTTP version: %s", parts[0].c_str());
    return false;
  }

  try {
    response_->set_status(std::stoi(parts[1]));
  } catch (const std::exception&) {
    LOG_ERRO("Invalid HTTP status code: %s", parts[1].c_str());
    return false;
  }

  if (parts.size() > 2) {
    response_->set_reason(parts[2]);
  }

  return true;
}

bool ResponseParser::ParseContent(const char* data, std::size_t length) {
  if (ignore_body_) {
    Finish();
    return true;
  }
  return Parser::ParseContent(data, length);
}

}  // namespace webcc
