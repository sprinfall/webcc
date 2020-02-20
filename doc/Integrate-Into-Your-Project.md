# Integrate Into Your Project

## Integrate with CMake

If your project also uses CMake as the build tool, it's highly recommended to integrate *Webcc* with source code. Just copy the `webcc` sub-folder to the side of your project, add the configure options to your top `CMakeLists.txt`, then include it by the following statement:

```cmake
add_subdirectory(webcc)
```

One of the difficulties is the dependency of third-party libraries, like `zlib` and `openssl`. It's not a big deal for Linux since they can be installed easily using the system package manager (`apt-get` for Ubuntu).

For Windows, you should copy `third_pary\src\zlib`, which is the source code of zlib with the official `CMakeLists.txt`, to some place of your project. Download OpenSSL and install it. See [Build Instructions](Build-Instructions.md) for more details.

## Integrate with Visual Studio

If you don't want to use CMake, you can copy the `webcc` sub-folder directly into your project to integrate.
But the following changes will be necessary:
- Create a file `config.h` by copying `config.h.example`, change the macros.
- Now create a static library project named _webcc_ in _Visual Studio_ with all the C++ files.
- Add the include path of _Boost_ in _Visual Studio_ for this project.
- Add webcc project's parent directory to the _Additional Including Directories_ of your solution. This is for including webcc headers with a prefix, e.g., `#include "webcc/http_client_session.h"`.
- Add the following definitions to the project properties page: **C/C++ -> Preprocessor -> Preprocessor Definitions**.

```
_WIN32_WINNT=0x0601
BOOST_ASIO_NO_DEPRECATED
```
The first one is needed by Asio. `0x0601` means Win7, should be OK even you develop with Win10.
