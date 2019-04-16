#include <iostream>

#include "webcc/http_client_session.h"
#include "webcc/logger.h"

#if (defined(WIN32) || defined(_WIN64))
// You need to set environment variable SSL_CERT_FILE properly to enable
// SSL verification.
bool kSslVerify = false;
#else
bool kSslVerify = true;
#endif

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  webcc::HttpClientSession session;

  session.set_ssl_verify(kSslVerify);

  try {
    // Using request builder:
    auto r = session.Request(webcc::HttpRequestBuilder{}.Get().
                             Url("http://httpbin.org/get").
                             Query("key1", "value1").
                             Query("key2", "value2").
                             Header("Accept", "application/json")
                             ());

    std::cout << r->content() << std::endl;
    
    // Using shortcut:
    r = session.Get("http://httpbin.org/get",
                    { "key1", "value1", "key2", "value2" },
                    { "Accept", "application/json" });

    std::cout << r->content() << std::endl;

#if WEBCC_ENABLE_SSL

    // HTTPS support.

    r = session.Get("https://httpbin.org/get");

    std::cout << r->content() << std::endl;

#endif  // WEBCC_ENABLE_SSL

  } catch (const webcc::Exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
