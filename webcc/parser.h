#ifndef WEBCC_PARSER_H_
#define WEBCC_PARSER_H_

#include <string>

#include "boost/filesystem/fstream.hpp"

#include "webcc/common.h"
#include "webcc/globals.h"

namespace webcc {

class Message;

// -----------------------------------------------------------------------------

class ParseHandler {
public:
  // If |stream| is true, the data will be streamed to a temp file, and the
  // body of the message will be FileBody instead of StringBody.
  ParseHandler(Message* message, bool stream = false);

  ~ParseHandler();

  std::size_t content_length() const {
    return content_length_;
  }

  void OnStartLine(const std::string& start_line);

  void OnContentLength(std::size_t content_length);

  void OnHeader(Header&& header);

  void AddContent(const char* data, std::size_t count);

  void AddContent(const std::string& data);

  bool IsFixedContentFull() const;

  bool Finish();

private:
  bool IsCompressed() const;

private:
  Message* message_;

  std::size_t content_length_;
  std::string content_;

  bool stream_;
  std::size_t streamed_size_;
  boost::filesystem::ofstream ofstream_;
  Path temp_path_;
};

// -----------------------------------------------------------------------------

// HTTP request and response parser.
class Parser {
public:
  Parser();
  virtual ~Parser() = default;

  Parser(const Parser&) = delete;
  Parser& operator=(const Parser&) = delete;

  void Init(Message* message, bool stream = false);

  bool finished() const {
    return finished_;
  }

  bool Parse(const char* data, std::size_t length);

protected:
  // Reset for next parse.
  void Reset();

  // Parse headers from pending data.
  // Return false only on syntax errors.
  bool ParseHeaders();

  // Get next line (using delimiter CRLF) from the pending data.
  // The line will not contain a trailing CRLF.
  // If |erase| is true, the line, as well as the trailing CRLF, will be erased
  // from the pending data.
  bool GetNextLine(std::size_t off, std::string* line, bool erase);

  virtual bool ParseStartLine(const std::string& line) = 0;

  bool ParseHeaderLine(const std::string& line);

  virtual bool ParseContent(const char* data, std::size_t length);

  bool ParseFixedContent(const char* data, std::size_t length);

  bool ParseChunkedContent(const char* data, std::size_t length);
  bool ParseChunkSize();

  // Return false if the compressed content cannot be decompressed.
  bool Finish();

protected:
  std::unique_ptr<ParseHandler> handler_;

  // Data waiting to be parsed.
  std::string pending_data_;

  // Temporary data and helper flags for parsing.
  ContentType content_type_;
  bool start_line_parsed_;
  bool content_length_parsed_;
  bool header_ended_;
  bool chunked_;
  std::size_t chunk_size_;
  bool finished_;
};

}  // namespace webcc

#endif  // WEBCC_PARSER_H_
