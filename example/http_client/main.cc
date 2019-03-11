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

void GetBoostOrgLicense(HttpClientSession& session) {
  try {
    auto r = session.Request(HttpRequestArgs{ "GET" }.
                             url("https://www.boost.org/LICENSE_1_0.txt").
                             ssl_verify(kSslVerify));

    std::cout << r->content() << std::endl;

  } catch (const webcc::Exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }
}

// TODO
void Test(HttpClientSession& session) {
  HttpResponsePtr r;

  // ---------------------------------------------------------------------------

  r = session.Request(HttpRequestArgs{ "GET" }.
                      url("http://httpbin.org/get").  // moved
                      parameters({ "key1", "value1", "key2", "value2" }).  // moved
                      headers({ "Accept", "application/json" }).  // moved
                      buffer_size(1000));

  std::cout << r->content() << std::endl;

  // ---------------------------------------------------------------------------

  // If you want to create the args object firstly, there'll be an extra call
  // to its move constructor.
  //   - constructor: HttpRequestArgs{ "GET" }
  //   - move constructor: auto args = ...

  auto args = HttpRequestArgs{"GET"}.
      url("http://httpbin.org/get").
      parameters({ "key1", "value1", "key2", "value2" }).
      headers({ "Accept", "application/json" }).
      buffer_size(1000);

  r = session.Request(std::move(args));

  // ---------------------------------------------------------------------------
  // Use pre-defined wrappers.

  r = session.Get("http://httpbin.org/get",
                  { "key1", "value1", "key2", "value2" },
                  { "Accept", "application/json" },
                  HttpRequestArgs{}.buffer_size(1000));

  // ---------------------------------------------------------------------------
  // HTTPS is auto-detected from the URL schema.

  try {
    r = session.Post("httpt://httpbin.org/post", "{ 'key': 'value' }", true,
                     {"Accept", "application/json"},
                     HttpRequestArgs{}.ssl_verify(false).buffer_size(1000));

    std::cout << r->status() << std::endl << r->content() << std::endl;

  } catch (const webcc::Exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }
}

void TestKeepAlive1(HttpClientSession& session) {
  try {
    auto r = session.Request(HttpRequestArgs{ "GET" }.
                             url("http://httpbin.org/get").
                             parameters({ "key1", "value1", "key2", "value2" }).
                             headers({ "Accept", "application/json" }).
                             buffer_size(1000));

    std::cout << r->content() << std::endl;

  } catch (const Exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }
}

void TestKeepAlive2(HttpClientSession& session) {
  try {
    auto r = session.Request(webcc::HttpRequestArgs("GET").
                             url("https://api.github.com/events").
                             ssl_verify(false).buffer_size(1500));

    //std::cout << r->content() << std::endl;

  } catch (const Exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }
}

void TestKeepAlive3(HttpClientSession& session) {
  try {
    auto r = session.Request(webcc::HttpRequestArgs("GET").
                             url("https://www.boost.org/LICENSE_1_0.txt").
                             ssl_verify(false));

    //std::cout << r->content() << std::endl;

  } catch (const Exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }
}

void TestKeepAlive4(HttpClientSession& session) {
  try {
    auto r = session.Request(webcc::HttpRequestArgs("GET").
                             url("https://www.google.com").
                             ssl_verify(false));

    //std::cout << r->content() << std::endl;

  } catch (const Exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }
}

// -----------------------------------------------------------------------------

int main() {
  WEBCC_LOG_INIT("", LOG_CONSOLE);

  HttpClientSession session;

  GetBoostOrgLicense(session);

  //TestKeepAlive1(session);
  //TestKeepAlive1(session);

  return 0;
}
