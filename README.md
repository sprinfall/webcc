# webcc

A lightweight C++ REST and SOAP client and server library based on *Boost.Asio*.

Please see the `doc` folder for tutorials and `example` folder for examples.

## Build Instructions

A lot of C++11 features are used, e.g., `std::move`. But C++14 is not required. It means that you can still build `webcc` using VS2013.

[CMake 3.1.0+](https://cmake.org/) is required as the build system. But if you don't use CMake, you can just copy the `src/webcc` folder to your own project then manage it by yourself.

[C++ Boost](https://www.boost.org/) should be 1.66+ because Asio made some broken changes to the API in 1.66.

### Build Options

The following CMake options determine how you build the projects. They are quite self-explanatory.

```cmake
option(WEBCC_ENABLE_LOG "Enable console logger?" ON)
option(WEBCC_ENABLE_SOAP "Enable SOAP support (need pugixml)?" ON)
option(WEBCC_BUILD_UNITTEST "Build unit test?" ON)
option(WEBCC_BUILD_REST_EXAMPLE "Build REST example?" ON)
option(WEBCC_BUILD_SOAP_EXAMPLE "Build SOAP example?" ON)
```

If `WEBCC_ENABLE_SOAP` is `ON`, **pugixml** (already included) is used to parse and compose XML strings.
