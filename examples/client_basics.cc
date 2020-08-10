#include <cassert>
#include <iostream>

#include "webcc/client_session.h"
#include "webcc/logger.h"

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  webcc::ClientSession session;

  session.Accept("application/json");

  webcc::ResponsePtr r;

  try {
    r = session.Send(webcc::RequestBuilder{}.
                     Get("http://httpbin.org/get").
                     Query("name", "Adam Gu", /*encode*/true).Date()
                     ());

    assert(r->status() == webcc::Status::kOK);
    assert(!r->data().empty());

    r = session.Send(webcc::RequestBuilder{}.
                     Post("http://httpbin.org/post").
                     Body("{'name'='Adam', 'age'=20}").Json().Utf8()
                     ());

    assert(r->status() == webcc::Status::kOK);
    assert(!r->data().empty());

#if WEBCC_ENABLE_SSL

    r = session.Send(webcc::RequestBuilder{}.
                     Get("https://httpbin.org/get")
                     ());

    assert(r->status() == webcc::Status::kOK);
    assert(!r->data().empty());

#endif  // WEBCC_ENABLE_SSL

  } catch (const webcc::Error& error) {
    std::cerr << error << std::endl;
    return 1;
  }

  return 0;
}
