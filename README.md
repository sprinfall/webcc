# webcc

C++ client and server library for HTTP, REST and SOAP based on *Boost.Asio*.

Please turn to our [Wiki](https://github.com/sprinfall/webcc/wiki) for more tutorials and guides, or just follow the links below:

- [Integrate Into Your Project](https://github.com/sprinfall/webcc/wiki/Integrate-Into-Your-Project)
- [Logging](https://github.com/sprinfall/webcc/wiki/Logging)
- [SOAP Client Tutorial](https://github.com/sprinfall/webcc/wiki/SOAP-Client-Tutorial)
- [SOAP Server Tutorial](https://github.com/sprinfall/webcc/wiki/SOAP-Server-Tutorial)
- [REST Server Tutorial](https://github.com/sprinfall/webcc/wiki/REST-Server-Tutorial)

## Quickstart

A complete client example: 
```cpp
#include <iostream>

#include "webcc/http_client_session.h"
#include "webcc/logger.h"

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  try {
    webcc::HttpClientSession session;

    auto r = session.Get("http://httpbin.org/get");

    std::cout << r->content() << std::endl;

  } catch (const webcc::Exception& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
```

List GitHub public events:
```cpp
  auto r = session.Get("https://api.github.com/events");

  // Parse r->content() to JSON object...
```

For more examples, see the Wiki tutorials.

## Build Instructions

### General

A lot of C++11 features are used, e.g., `std::move`.

VS2015 and above is required for building `webcc` on Windows. No support for VS2013 any more!

[CMake 3.1.0+](https://cmake.org/) is used as the build system. But if you don't use CMake, you can just copy the `src/webcc` folder to your own project then manage it by yourself, though some changes are needed to make it work. See [Wiki/Integrate Into Your Project]( https://github.com/sprinfall/webcc/wiki/Integrate-Into-Your-Project) for more details.

[C++ Boost](https://www.boost.org/) should be 1.66+ because Asio made some broken changes to the API since 1.66.

OpenSSL is required for HTTPS support. Already included in the `third_party` folder for Windows.

Zlib is required for compressing and decompressing HTTP requests and responses.

### Build Options

The following CMake options determine how you build the projects. They are quite self-explanatory.

```cmake
option(WEBCC_ENABLE_SOAP "Enable SOAP support (need pugixml)?" ON)
option(WEBCC_ENABLE_UNITTEST "Build unit test?" ON)
option(WEBCC_ENABLE_EXAMPLES "Build examples?" ON)

set(WEBCC_ENABLE_LOG 1 CACHE STRING "Enable logging? (0:OFF, 1:ON)")
set(WEBCC_LOG_LEVEL 2 CACHE STRING "Log level (0:VERB, 1:INFO, 2:WARN, 3:ERRO or 4:FATA)")
```

Options `WEBCC_ENABLE_LOG` and `WEBCC_LOG_LEVEL` together define how logging behaves. See [Wiki/Logging](https://github.com/sprinfall/webcc/wiki/Logging) for more details.

If `WEBCC_ENABLE_SOAP` is `1`, **pugixml** (already included) is used to parse and compose XML strings.

### Build On Linux

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
    -DWEBCC_ENABLE_SOAP=ON \
    -DWEBCC_ENABLE_UNITTEST=OFF \
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
