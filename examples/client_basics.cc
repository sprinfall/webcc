#include <iostream>

#include "webcc/client_session.h"
#include "webcc/logger.h"

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  webcc::ClientSession session;

  try {
    auto r = session.Request(webcc::RequestBuilder{}.
                             Get("http://httpbin.org/get").
                             Query("key1", "value1").
                             Query("key2", "value2").
                             Date().
                             Header("Accept", "application/json")
                             ());

    r = session.Get("http://httpbin.org/get",
                    { "key1", "value1", "key2", "value2" },
                    { "Accept", "application/json" });

    r = session.Request(webcc::RequestBuilder{}.
                        Post("http://httpbin.org/post").
                        Body("{'name'='Adam', 'age'=20}").
                        Json().Utf8()
                        ());

#if WEBCC_ENABLE_SSL

    r = session.Get("https://httpbin.org/get");

#endif  // WEBCC_ENABLE_SSL

  } catch (const webcc::Error& error) {
    std::cerr << error << std::endl;
  }

  return 0;
}
