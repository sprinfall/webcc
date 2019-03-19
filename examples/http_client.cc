#include <iostream>

#include "webcc/http_client_session.h"
#include "webcc/logger.h"

using namespace webcc;

// -----------------------------------------------------------------------------

#if (defined(WIN32) || defined(_WIN64))
// You need to set environment variable SSL_CERT_FILE properly to enable
// SSL verification.
bool kSslVerify = false;
#else
bool kSslVerify = true;
#endif

// -----------------------------------------------------------------------------
// Examples

void ExampleBasic() {
  HttpClientSession session;

  auto r = session.Request(HttpRequestArgs{"GET"}
                               .url("http://httpbin.org/get")
                               .parameters({"key1", "value1", "key2", "value2"})
                               .headers({"Accept", "application/json"})
                               .buffer_size(1000));

  std::cout << r->content() << std::endl;
}

// If you want to create the args object firstly, there might be an extra
// call to its move constructor (maybe only for MSVC).
//   - constructor: HttpRequestArgs{ "GET" }
//   - move constructor: auto args = ...
void ExampleArgs() {
  HttpClientSession session;

  auto args = HttpRequestArgs{"GET"}
                  .url("http://httpbin.org/get")
                  .parameters({"key1", "value1", "key2", "value2"})
                  .headers({"Accept", "application/json"})
                  .buffer_size(1000);

  // Note the std::move().
  session.Request(std::move(args));
}

// Use pre-defined wrappers.
void ExampleWrappers() {
  HttpClientSession session;

  session.Get("http://httpbin.org/get", {"key1", "value1", "key2", "value2"},
              {"Accept", "application/json"},
              HttpRequestArgs{}.buffer_size(1000));

  session.Post("http://httpbin.org/post", "{ 'key': 'value' }", true,
               {"Accept", "application/json"});
}

// HTTPS is auto-detected from the URL scheme.
void ExampleHttps() {
  HttpClientSession session;

  auto r = session.Request(HttpRequestArgs{"GET"}
                               .url("https://httpbin.org/get")
                               .parameters({"key1", "value1", "key2", "value2"})
                               .headers({"Accept", "application/json"})
                               .ssl_verify(kSslVerify));

  std::cout << r->content() << std::endl;
}

// Example for testing Keep-Alive connection.
//
// Boost.org doesn't support persistent connection so always includes
// "Connection: Close" header in the response.
// Both Google and GitHub support persistent connection but they don't like
// to include "Connection: Keep-Alive" header in the responses.
// 
//   ExampleKeepAlive("http://httpbin.org/get");
//   ExampleKeepAlive("https://www.boost.org/LICENSE_1_0.txt");
//   ExampleKeepAlive("https://www.google.com");
//   ExampleKeepAlive("https://api.github.com/events");
//
void ExampleKeepAlive(const std::string& url) {
  HttpClientSession session;

  // Keep-Alive
  session.Request(webcc::HttpRequestArgs("GET").url(url).
                  ssl_verify(kSslVerify));

  // Close
  session.Request(webcc::HttpRequestArgs("GET").url(url).
                  ssl_verify(kSslVerify).
                  headers({ "Connection", "Close" }));

  // Keep-Alive
  session.Request(webcc::HttpRequestArgs("GET").url(url).
                  ssl_verify(kSslVerify));
}

void ExampleCompression() {
  HttpClientSession session;

  auto r = session.Get("http://httpbin.org/gzip");
  std::cout << r->content() << std::endl;

  r = session.Get("http://httpbin.org/deflate");
  std::cout << r->content() << std::endl;
}

int main() {
  WEBCC_LOG_INIT("", LOG_CONSOLE);

  // Note that the exception handling is mandatory.
  try {

    ExampleBasic();

  } catch (const Exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
