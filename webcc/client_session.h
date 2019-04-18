#ifndef WEBCC_CLIENT_SESSION_H_
#define WEBCC_CLIENT_SESSION_H_

#include <string>
#include <vector>

#include "webcc/client_pool.h"
#include "webcc/request_builder.h"
#include "webcc/response.h"

namespace webcc {

// HTTP requests session providing connection-pooling, configuration and more.
// A session shouldn't be shared by multiple threads. Please create a new
// session for each thread instead.
class ClientSession {
public:
  ClientSession();

  ~ClientSession() = default;

  void set_ssl_verify(bool ssl_verify) {
    ssl_verify_ = ssl_verify;
  }

  void set_buffer_size(std::size_t buffer_size) {
    buffer_size_ = buffer_size;
  }

  void set_timeout(int timeout) {
    if (timeout > 0) {
      timeout_ = timeout;
    }
  }

  void SetHeader(const std::string& key, const std::string& value) {
    headers_.Set(key, value);
  }

  void set_content_type(const std::string& content_type) {
    content_type_ = content_type;
  }

  void set_charset(const std::string& charset) {
    charset_ = charset;
  }

  // Set authorization.
  void Auth(const std::string& type, const std::string& credentials);

  // Set Basic authorization.
  void AuthBasic(const std::string& login, const std::string& password);

  // Set Token authorization.
  void AuthToken(const std::string& token);

  // Send a request.
  // Please use RequestBuilder to build the request.
  ResponsePtr Request(RequestPtr request);

  // Shortcut for GET request.
  ResponsePtr Get(const std::string& url,
                  const std::vector<std::string>& parameters = {},
                  const std::vector<std::string>& headers = {});

  // Shortcut for POST request.
  ResponsePtr Post(const std::string& url, std::string&& data, bool json,
                   const std::vector<std::string>& headers = {});

  // Shortcut for PUT request.
  ResponsePtr Put(const std::string& url, std::string&& data, bool json,
                  const std::vector<std::string>& headers = {});

  // Shortcut for DELETE request.
  ResponsePtr Delete(const std::string& url,
                     const std::vector<std::string>& headers = {});

  // Shortcut for PATCH request.
  ResponsePtr Patch(const std::string& url, std::string&& data, bool json,
                    const std::vector<std::string>& headers = {});

  // Post a file.
  ResponsePtr PostFile(const std::string& url, const std::string& name,
                       const Path& path,
                       const std::vector<std::string>& headers = {});

  // Post multiple files.
  ResponsePtr PostFiles(const std::string& url,
                        const std::map<std::string, Path>& paths,
                        const std::vector<std::string>& headers = {});

private:
  void InitHeaders();

  ResponsePtr Send(RequestPtr request);

private:
  // E.g., "application/json".
  std::string content_type_;

  // E.g., "utf-8".
  std::string charset_;

  // Additional headers for each request.
  Headers headers_;

  // Verify the certificate of the peer or not.
  bool ssl_verify_ = true;

  // The size of the buffer for reading response.
  // 0 means default value will be used.
  std::size_t buffer_size_ = 0;

  // Timeout in seconds for receiving response.
  int timeout_ = 0;

  // Connection pool for keep-alive.
  ClientPool pool_;
};

}  // namespace webcc

#endif  // WEBCC_CLIENT_SESSION_H_
