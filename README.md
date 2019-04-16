# webcc

C++ client and server library for HTTP and REST based on *Boost Asio*.

Please turn to our [Wiki](https://github.com/sprinfall/webcc/wiki) (under construction) for more tutorials and guides.

## Client API Examples

A complete client example: 
```cpp
#include <iostream>

#include "webcc/http_client_session.h"
#include "webcc/logger.h"

int main() {
  // Configure logger.
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);
  
  // Session provides convenient request APIs, stores request configurations
  // and manages persistent connenction.
  webcc::HttpClientSession session;

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
auto r = session.Request(webcc::HttpRequestBuilder{}.Get().
                         Url("http://httpbin.org/get")
                         ());
```
As you can see, a helper class named `HttpRequestBuilder` is used to chain the parameters and finally build (don't miss the `()` operator) a request object.

Both the shortcut and `Request()` accept URL query parameters:

```cpp
// Query parameters are passed using a std::vector. 
session.Get("http://httpbin.org/get", { "key1", "value1", "key2", "value2" });

session.Request(webcc::HttpRequestBuilder{}.Get().
                Url("http://httpbin.org/get").
                Query("key1", "value1").
                Query("key2", "value2")
                ());
```

Adding additional headers is also easy:
```cpp
r = session.Get("http://httpbin.org/get",
                {"key1", "value1", "key2", "value2"},
                {"Accept", "application/json"});  // Also a std::vector
                
session.Request(webcc::HttpRequestBuilder{}.Get().
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

## Build Instructions

### General

A lot of C++11 features are used, e.g., `std::move`.

VS2013 and above is required for building `webcc` on Windows.

[CMake 3.1.0+](https://cmake.org/) is used as the build system. But if you don't use CMake, you can just copy the `src/webcc` folder to your own project then manage it by yourself, though some changes are needed to make it work. See [Wiki/Integrate Into Your Project]( https://github.com/sprinfall/webcc/wiki/Integrate-Into-Your-Project) for more details.

[C++ Boost](https://www.boost.org/) should be 1.66+ because Asio made some broken changes to the API since 1.66.

OpenSSL is required for HTTPS support. See `WEBCC_ENABLE_SSL` option.

Zlib is required for compressing and decompressing HTTP requests and responses. Already included in the `third_party` folder for Windows.

### Build Options

The following CMake options determine how you build the projects. They are quite self-explanatory.

```cmake
option(WEBCC_ENABLE_TEST "Build test?" ON)
option(WEBCC_ENABLE_UNITTEST "Build unit test?" ON)
option(WEBCC_ENABLE_EXAMPLES "Build examples?" ON)

set(WEBCC_ENABLE_LOG 1 CACHE STRING "Enable logging? (1:Yes, 0:No)")
set(WEBCC_LOG_LEVEL 2 CACHE STRING "Log level (0:VERB, 1:INFO, 2:WARN, 3:ERRO or 4:FATA)")

set(WEBCC_ENABLE_SSL 0 CACHE STRING "Enable SSL/HTTPS (need OpenSSL)? (1:Yes, 0:No)")
```

Options `WEBCC_ENABLE_LOG` and `WEBCC_LOG_LEVEL` together define how logging behaves. See [Wiki/Logging](https://github.com/sprinfall/webcc/wiki/Logging) for more details.

You should install OpenSSL development files before try to enable `WEBCC_ENABLE_SSL`.

### Build on Linux

Create a build folder under the root (or any other) directory, and `cd` to it:
```bash
mkdir build
cd build
```
Generate Makefiles with the following command:
```bash
cmake -G"Unix Makefiles" \
    -DCMAKE_INSTALL_PREFIX=~ \
    -DWEBCC_ENABLE_LOG=1 \
    -DWEBCC_LOG_LEVEL=2 \
    -DWEBCC_ENABLE_SSL=ON \
    -DWEBCC_ENABLE_UNITTEST=ON \
    -DWEBCC_ENABLE_UNITTEST=ON \
    -DWEBCC_ENABLE_EXAMPLES=ON \
    ..
```
Feel free to change the build options.
CMake variable `CMAKE_INSTALL_PREFIX` defines where to install the output library and header files. The default is `/usr/local`.

If everything is OK, then you can build with `make`:
```bash
$ make -j4  # or -j8, depending on how many CPU cores you have.
```

Then install:
```bash
$ make install
```
