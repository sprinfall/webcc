// HttpBin (http://httpbin.org/) client example.
//
// You request to different endpoints, and it returns information about what
// was in the request.
//
// E.g., request:
//     > GET /get HTTP/1.1
//     > Host: httpbin.org:80
//     > User-Agent: Webcc/0.1.0
//     >
// Response:
//     > HTTP/1.1 200 OK
//     > Connection: keep-alive
//     > Server: gunicorn/19.9.0
//     > Content-Type: application/json
//     > Content-Length: 191
//     > Access-Control-Allow-Origin: *
//     > Access-Control-Allow-Credentials: true
//     > Via: 1.1 vegur
//     >
//     > {
//     >   "args": {},
//     >   "headers": {
//     >     "Connection": "close",
//     >     "Host": "httpbin.org",
//     >     "User-Agent": "Webcc/0.1.0"
//     >   },
//     >   "origin": "198.55.94.81",
//     >   "url": "http://httpbin.org/get"
//     > }
//     >
// As you can see, the request information is returned in JSON format.

#include <iostream>

#include "webcc/http_client.h"
#include "webcc/logger.h"

void Test() {
  webcc::HttpRequest request;
  request.set_method(webcc::kHttpGet);
  request.set_url("/get");
  request.set_host("httpbin.org"/*, "80"*/);
  request.Make();

  webcc::HttpClient client;
  if (client.Request(request)) {
    std::cout << client.response()->content() << std::endl;
  } else {
    std::cout << webcc::DescribeError(client.error());
    if (client.timed_out()) {
      std::cout << " (timed out)";
    }
    std::cout << std::endl;
  }
}

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  Test();

  return 0;
}
