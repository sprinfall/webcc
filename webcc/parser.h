#ifndef WEBCC_PARSER_H_
#define WEBCC_PARSER_H_

#include <fstream>
#include <string>

#include "webcc/common.h"
#include "webcc/globals.h"

namespace webcc {

class Message;

// -----------------------------------------------------------------------------

class BodyHandler {
public:
  explicit BodyHandler(Message* message) : message_(message) {
  }

  BodyHandler(const BodyHandler&) = delete;
  BodyHandler& operator=(const BodyHandler&) = delete;

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
  std::ofstream ofstream_;
  sfs::path temp_path_;
};

// -----------------------------------------------------------------------------

// HTTP request/response parser.
class Parser {
public:
  Parser() = default;

  Parser(const Parser&) = delete;
  Parser& operator=(const Parser&) = delete;

  virtual ~Parser() = default;

  void Init(Message* message);

  bool finished() const {
    return finished_;
  }

  // If the headers part has been parsed or not.
  bool header_ended() const {
  return header_ended_;
  }

  // Get the length of the headers part.
  // Available after the headers have been parsed (see header_ended()).
  std::size_t header_length() const {
    return header_length_;
  }

  // The content length parsed from the Content-Length header.
  // Return kInvalidLength if the content is chunked.
  std::size_t content_length() const {
    return content_length_;
  }

  // Parse the given length of data.
  // Return false if the parsing is failed.
  bool Parse(const char* data, std::size_t length);

protected:
  // Parse headers from pending data.
  // Return false only on syntax errors.
  bool ParseHeaders();

  // Called when headers just parsed.
  // Return false if something is wrong.
  virtual bool OnHeadersEnd() = 0;

  void CreateBodyHandler();

  // Get next line (using delimiter CRLF) from the pending data.
  // The line will not contain a trailing CRLF.
  // If `erase` is true, the line, as well as the trailing CRLF, will be erased
  // from the pending data.
  bool GetNextLine(std::size_t off, std::string* line, bool erase);

  virtual bool ParseStartLine(const std::string& line) = 0;

  bool ParseHeaderLine(const std::string& line);

  // Parse the given length of data.
  virtual bool ParseContent(const char* data, std::size_t length);

  bool ParseFixedContent(const char* data, std::size_t length);

  bool ParseChunkedContent(const char* data, std::size_t length);

  bool ParseChunkSize(const std::string& line);

  bool IsFixedContentFull() const;

  // Return false if the compressed content cannot be decompressed.
  bool Finish();

protected:
  Message* message_ = nullptr;

  std::unique_ptr<BodyHandler> body_handler_;

  // Data streaming or not.
  bool stream_ = false;

  // Data waiting to be parsed.
  std::string pending_data_;

  // The length of the headers part.
  std::size_t header_length_ = 0;

  // Temporary data and helper flags for parsing.
  std::size_t content_length_ = kInvalidLength;
  ContentType content_type_;
  bool start_line_parsed_ = false;
  bool content_length_parsed_ = false;
  bool header_ended_ = false;
  bool chunked_ = false;
  std::size_t chunk_size_ = kInvalidLength;
  bool finished_ = false;
};

}  // namespace webcc

#endif  // WEBCC_PARSER_H_
