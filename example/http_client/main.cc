#include <iostream>

#include "webcc/http_client_session.h"  // TEST
#include "webcc/logger.h"

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  using namespace webcc;

  HttpResponsePtr r;

  HttpClientSession session;

#if 0
  r = session.Request(HttpRequestArgs("GET")
                      .url("http://httpbin.org/get")  // Moved
                      .parameters({ "key1", "value1", "key2", "value2" })  // Moved
                      .headers({ "Accept", "application/json" })  // Moved
                      .buffer_size(1000));

  std::cout << r->content() << std::endl;

  // If you want to create the args object firstly, there'll be an extra call
  // to its move constructor.
  //   - constructor: HttpRequestArgs("GET")
  //   - move constructor: auto args = ... 
  auto args = HttpRequestArgs("GET")
      .url("http://httpbin.org/get")
      .parameters({ "key1", "value1", "key2", "value2" })
      .headers({ "Accept", "application/json" })
      .buffer_size(1000);

  r = session.Request(std::move(args));

  r = session.Get("http://httpbin.org/get",
                  { "key1", "value1", "key2", "value2" },
                  { "Accept", "application/json" },
                  HttpRequestArgs().buffer_size(1000));
#endif

  r = session.Post("http://httpbin.org/post", "{ 'key': 'value' }", true,
                   { "Accept", "application/json" },
                   HttpRequestArgs().buffer_size(1000));

  std::cout << r->content() << std::endl;

  return 0;
}
