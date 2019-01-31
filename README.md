# webcc

A lightweight C++ REST and SOAP client and server library based on *Boost.Asio*.

Please turn to our [Wiki](https://github.com/sprinfall/webcc/wiki) for more tutorials and guides, or just follow the links below:

- [Integrate Into Your Project](https://github.com/sprinfall/webcc/wiki/Integrate-Into-Your-Project)
- [Logging](https://github.com/sprinfall/webcc/wiki/Logging)
- [SOAP Client Tutorial](https://github.com/sprinfall/webcc/wiki/SOAP-Client-Tutorial)
- [SOAP Server Tutorial](https://github.com/sprinfall/webcc/wiki/SOAP-Server-Tutorial)
- [REST Server Tutorial](https://github.com/sprinfall/webcc/wiki/REST-Server-Tutorial)

## Build Instructions

A lot of C++11 features are used, e.g., `std::move`. But C++14 is not required.
(It means that you can still build `webcc` using VS2013 on Windows.)

[CMake 3.1.0+](https://cmake.org/) is required as the build system. But if you don't use CMake, you can just copy the `src/webcc` folder to your own project then manage it by yourself, though some changes are needed to make it work. See [Wiki/Integrate Into Your Project]( https://github.com/sprinfall/webcc/wiki/Integrate-Into-Your-Project) for more details.

[C++ Boost](https://www.boost.org/) should be 1.66+ because Asio made some broken changes to the API in 1.66.

### Build Options

The following CMake options determine how you build the projects. They are quite self-explanatory.

```cmake
option(WEBCC_ENABLE_SOAP "Enable SOAP support (need pugixml)?" ON)
option(WEBCC_ENABLE_SSL "Enable SSL/HTTPS support (need OpenSSL)?" OFF)
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
    -DWEBCC_ENABLE_SSL=OFF \
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
