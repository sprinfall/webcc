#ifndef WEBCC_PARSER_H_
#define WEBCC_PARSER_H_

#include <string>

#include "boost/filesystem/fstream.hpp"

#include "webcc/common.h"
#include "webcc/globals.h"

namespace webcc {

class Message;

// -----------------------------------------------------------------------------

class ParseHandlerBase {
public:
  ParseHandlerBase(Message* message);

  virtual ~ParseHandlerBase() = default;

  virtual bool Init() = 0;

  std::size_t content_length() const {
    return content_length_;
  }

  void OnStartLine(const std::string& start_line);

  void OnHeader(Header&& header);

  virtual void OnContentLength(std::size_t content_length);

  virtual void AddContent(const char* data, std::size_t count) = 0;

  virtual void AddContent(const std::string& data) = 0;

  virtual bool IsFixedContentFull() const = 0;

  virtual bool Finish() = 0;

protected:
  bool IsCompressed() const;

protected:
  Message* message_;
  std::size_t content_length_;
};

class ParseHandler : public ParseHandlerBase {
public:
  explicit ParseHandler(Message* message);

  ~ParseHandler() override = default;

  bool Init() override {
    return true;
  }

  void OnContentLength(std::size_t content_length) override;

  void AddContent(const char* data, std::size_t count) override;
  void AddContent(const std::string& data) override;

  bool IsFixedContentFull() const override;
  bool Finish() override;

private:
  std::string content_;
};

// If |stream| is true, the data will be streamed to a temp file, and the
// body of the message will be FileBody instead of StringBody.
class StreamedParseHandler : public ParseHandlerBase {
public:
  explicit StreamedParseHandler(Message* message);

  ~StreamedParseHandler() override = default;

  // Generate a temp file.
  bool Init() override;

  void AddContent(const char* data, std::size_t count) override;
  void AddContent(const std::string& data) override;

  bool IsFixedContentFull() const override;
  bool Finish() override;

private:
  std::size_t streamed_size_ = 0;
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

  bool Init(Message* message, bool stream = false);

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
  std::unique_ptr<ParseHandlerBase> handler_;

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
