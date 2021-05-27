// A client posting urlencoded form data.

#include <iostream>

#include "webcc/client_session.h"
#include "webcc/logger.h"

int main(int argc, char* argv[]) {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  webcc::ClientSession session;

  session.SetContentType("application/x-www-form-urlencoded", "utf8");

  std::string url = "http://httpbin.org/post";

  try {
    // Don't use RequestBuilder::Query() which is dedicated to GET.
    webcc::UrlQuery query;
    query.Add("key1", "value1");
    query.Add("key2", "value2");
    // ...

    auto r = session.Send(
        webcc::RequestBuilder{}.Post(url).Body(query.ToString())());

    std::cout << r->data() << std::endl;

  } catch (const webcc::Error& error) {
    std::cout << error << std::endl;
  }

  return 0;
}
