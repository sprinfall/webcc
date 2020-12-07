#ifndef WEBCC_PARSER_H_
#define WEBCC_PARSER_H_

#include <fstream>
#include <string>

#include "boost/filesystem/fstream.hpp"
#include "boost/filesystem/path.hpp"

#include "webcc/common.h"
#include "webcc/globals.h"

namespace webcc {

class Message;

// -----------------------------------------------------------------------------

class BodyHandler {
public:
  explicit BodyHandler(Message* message) : message_(message) {
  }

  virtual ~BodyHandler() = default;

  virtual void AddContent(const char* data, std::size_t count) = 0;

  virtual void AddContent(const std::string& data) = 0;

  virtual std::size_t GetContentLength() const = 0;

  virtual bool Finish() = 0;

protected:
  bool IsCompressed() const;

protected:
  Message* message_;
};

// -----------------------------------------------------------------------------

class StringBodyHandler : public BodyHandler {
public:
  explicit StringBodyHandler(Message* message) : BodyHandler(message) {
  }

  ~StringBodyHandler() override = default;

  void AddContent(const char* data, std::size_t count) override;
  void AddContent(const std::string& data) override;

  std::size_t GetContentLength() const override {
    return content_.size();
  }

  bool Finish() override;

private:
  std::string content_;
};

// -----------------------------------------------------------------------------

class FileBodyHandler : public BodyHandler {
public:
  // NOTE: Might throw Error::kFileError.
  explicit FileBodyHandler(Message* message) : BodyHandler(message) {
  }

  ~FileBodyHandler() override = default;

  // Open a temp file for data streaming.
  bool OpenFile();

  void AddContent(const char* data, std::size_t count) override;
  void AddContent(const std::string& data) override;

  std::size_t GetContentLength() const override {
    return streamed_size_;
  }

  bool Finish() override;

private:
  std::size_t streamed_size_ = 0;
  boost::filesystem::ofstream ofstream_;
  boost::filesystem::path temp_path_;
};

// -----------------------------------------------------------------------------

// HTTP request and response parser.
class Parser {
public:
  Parser();
  virtual ~Parser() = default;

  Parser(const Parser&) = delete;
  Parser& operator=(const Parser&) = delete;

  void Init(Message* message);

  bool finished() const {
    return finished_;
  }

  bool Parse(const char* data, std::size_t length);

protected:
  void Reset();

  // Parse headers from pending data.
  // Return false only on syntax errors.
  bool ParseHeaders();

  // Called when headers just parsed.
  // Return false if something is wrong.
  virtual bool OnHeadersEnd() = 0;

  void CreateBodyHandler();

  // Get next line (using delimiter CRLF) from the pending data.
  // The line will not contain a trailing CRLF.
  // If |erase| is true, the line, as well as the trailing CRLF, will be erased
  // from the pending data.
  bool GetNextLine(std::size_t off, std::string* line, bool erase);

  virtual bool ParseStartLine(const std::string& line) = 0;

  bool ParseHeaderLine(const std::string& line);

  // Parse the given length of data.
  virtual bool ParseContent(const char* data, std::size_t length);

  bool ParseFixedContent(const char* data, std::size_t length);

  bool ParseChunkedContent(const char* data, std::size_t length);
  bool ParseChunkSize();

  bool IsFixedContentFull() const;

  // Return false if the compressed content cannot be decompressed.
  bool Finish();

protected:
  Message* message_;

  std::unique_ptr<BodyHandler> body_handler_;

  // Data streaming or not.
  bool stream_;

  // Data waiting to be parsed.
  std::string pending_data_;

  // Temporary data and helper flags for parsing.
  std::size_t content_length_;
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
