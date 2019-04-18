# webcc

C++ client and server library for HTTP and REST based on *Boost Asio*.

Please turn to our [Wiki](https://github.com/sprinfall/webcc/wiki) (under construction) for more tutorials and guides.

Wondering how to build Webcc? Click [Build Instructions](https://github.com/sprinfall/webcc/wiki/Build-Instructions).

## Client API Examples

A complete client example: 
```cpp
#include <iostream>

#include "webcc/client_session.h"
#include "webcc/logger.h"

int main() {
  // Configure logger.
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);
  
  // Session provides convenient request APIs, stores request configurations
  // and manages persistent connenction.
  webcc::ClientSession session;

  // Catch exceptions for error handling.
  try {
    // Send a HTTP GET request.
    auto r = session.Get("http://httpbin.org/get");

    // Print the response content data.
    std::cout << r->content() << std::endl;

  } catch (const webcc::Exception& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
```

The `Get()` method is nothing but a shortcut of `Request()`. Using `Request()` directly is more complicated:
```cpp
auto r = session.Request(webcc::RequestBuilder{}.Get().
                         Url("http://httpbin.org/get")
                         ());
```
As you can see, a helper class named `RequestBuilder` is used to chain the parameters and finally build (don't miss the `()` operator) a request object.

Both the shortcut and `Request()` accept URL query parameters:

```cpp
// Query parameters are passed using a std::vector. 
session.Get("http://httpbin.org/get", { "key1", "value1", "key2", "value2" });

session.Request(webcc::RequestBuilder{}.Get().
                Url("http://httpbin.org/get").
                Query("key1", "value1").
                Query("key2", "value2")
                ());
```

Adding additional headers is also easy:
```cpp
session.Get("http://httpbin.org/get",
            {"key1", "value1", "key2", "value2"},
            {"Accept", "application/json"});  // Also a std::vector
                
session.Request(webcc::RequestBuilder{}.Get().
                Url("http://httpbin.org/get").
                Query("key1", "value1").
                Query("key2", "value2").
                Header("Accept", "application/json")
                ());
```

Accessing HTTPS has no difference from HTTP:
```cpp
session.Get("https://httpbin.org/get");
```
*NOTE: The HTTPS/SSL support requires the build option `WEBCC_ENABLE_SSL` to be enabled.*

Listing GitHub public events is not a big deal:
```cpp
auto r = session.Get("https://api.github.com/events");
```
You can then parse `r->content()` to JSON object with your favorite JSON library. My choice for the examples is [jsoncpp](https://github.com/open-source-parsers/jsoncpp). But the library itself doesn't understand JSON nor require one. It's up to you to choose the most appropriate JSON library.
