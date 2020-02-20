# Build Instructions

## Dependencies

* [Boost](https://www.boost.org/) (asio, system, filesystem, date_time)
* [OpenSSL](https://www.openssl.org/) (HTTPS)
* [Zlib](https://www.zlib.net/) (GZIP compression)
* [Googletest/gtest](https://github.com/google/googletest) (automation and unit tests)
* [CMake](https://cmake.org/)

Googletest is included in `third_party/src`. No need to install.

OpenSSL and Zlib are **optional** since they could be disabled. See the build options below.

## Build Options

The following CMake options determine how you build the projects. They are quite self-explanatory.

```cmake
option(WEBCC_ENABLE_AUTOTEST "Build automation test?" OFF)
option(WEBCC_ENABLE_UNITTEST "Build unit test?" OFF)
option(WEBCC_ENABLE_EXAMPLES "Build examples?" OFF)

set(WEBCC_ENABLE_LOG 1 CACHE STRING "Enable logging? (1:Yes, 0:No)")
set(WEBCC_LOG_LEVEL 2 CACHE STRING "Log level (0:VERB, 1:INFO, 2:USER, 3:WARN or 4:ERRO)")

set(WEBCC_ENABLE_SSL 0 CACHE STRING "Enable SSL/HTTPS (need OpenSSL)? (1:Yes, 0:No)")
set(WEBCC_ENABLE_GZIP 0 CACHE STRING "Enable gzip compression (need Zlib)? (1:Yes, 0:No)")
```

### `WEBCC_ENABLE_AUTOTEST`

Automation test based on real servers (mostly [httpbin.org](http://httpbin.org/)).

### `WEBCC_ENABLE_LOG` and `WEBCC_LOG_LEVEL`

These two options define how logging behaves.
See [Logging](Logging.md) for more details.

### `WEBCC_ENABLE_SSL`

For HTTPS support (need OpenSSL).

### `WEBCC_ENABLE_GZIP`

For GZIP compression support (need Zlib).

## Platforms

- [Build on Windows](Build-on-Windows.md)
- [Build on Linux](Build-on-Linux.md)
- [Build on Mac](Build-on-Mac.md)
