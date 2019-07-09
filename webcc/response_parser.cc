#include "webcc/response_parser.h"

#include "boost/algorithm/string.hpp"

#include "webcc/logger.h"
#include "webcc/response.h"

namespace webcc {

// -----------------------------------------------------------------------------

namespace {

void SplitStartLine(const std::string& line, std::vector<std::string>* parts) {
  const char SPACE = ' ';

  std::size_t off = 0;
  std::size_t pos = 0;

  for (std::size_t i = 0; i < 2; ++i) {
    pos = line.find(SPACE, off);
    if (pos == std::string::npos) {
      break;
    }

    parts->push_back(line.substr(off, pos - off));
    off = pos + 1;

    for (; off < line.size() && line[off] == SPACE; ++off) {}
  }

  if (off < line.size()) {
    parts->push_back(line.substr(off));
  }
}

}  // namespace

// -----------------------------------------------------------------------------

ResponseParser::ResponseParser(Response* response)
    : Parser(response), response_(response) {
}

void ResponseParser::Init(Response* response) {
  Parser::Init(response);
  response_ = response;
}

bool ResponseParser::ParseStartLine(const std::string& line) {
  std::vector<std::string> parts;
  SplitStartLine(line, &parts);

  if (parts.size() != 3) {
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

  response_->set_reason(parts[2]);

  return true;
}

bool ResponseParser::ParseContent(const char* data, std::size_t length) {
  if (ignroe_body_) {
    Finish();
    return true;
  }
  return Parser::ParseContent(data, length);
}

}  // namespace webcc
