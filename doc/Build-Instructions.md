# Build Instructions

* [Overview](#overview)
* [Build on Ubuntu](#build-on-ubuntu)
* [Build on Windows](#build-on-windows)

## Overview

### Dependencies

* [Boost 1.74+](https://www.boost.org/) (for Asio)
* [OpenSSL](https://www.openssl.org/) (for HTTPS)
* [Zlib](https://www.zlib.net/) (for GZIP compression, **optional**)
* [Googletest](https://github.com/google/googletest) (for automation and unit tests, **optional**)
* [CMake](https://cmake.org/)

Zlib is **optional** since GZIP compression could be disabled. See the build options below.

Googletest is also **optional** unless you want to build the automation and unit tests.

## Build Options

The following CMake options determine how you build the projects. They are quite self-explanatory.

```cmake
option(WEBCC_BUILD_UNITTEST "Build unit test?" OFF)
option(WEBCC_BUILD_AUTOTEST "Build automation test?" OFF)
option(WEBCC_BUILD_EXAMPLES "Build examples?" ON)
option(WEBCC_BUILD_QT_EXAMPLES "Build Qt application examples?" OFF)

set(WEBCC_LOG_LEVEL 2 CACHE STRING "Log level (0:VERB, 1:INFO, 2:USER, 3:WARN, 4:ERRO, 5:NONE")
set(WEBCC_ENABLE_GZIP 0 CACHE STRING "Enable gzip compression (need Zlib)? (1:Yes, 0:No)")
```

Automation test is based on a real server: [httpbin.org](http://httpbin.org/) (it seems not accessible at times).

### Integration

Webcc doesn't support CMake Install right now.

I suggest to integrate it to your project simply by source code. Just copy the webcc sub-folder into your project and add the related CMake options. Please refer to: [https://github.com/sprinfall/webcc-integration](https://github.com/sprinfall/webcc-integration).

## Build on Ubuntu

*NOTE: Based on Ubuntu 20.04 LTS*

Please install `build-essential` which includes the C++ compiler and more:

```
$ sudo apt install build-essential
```
### CMake

Please refer to [https://apt.kitware.com/](https://apt.kitware.com/).

### OpenSSL

```
$ sudo apt install libssl-dev
```

### Zlib

```
$ sudo apt install zlib1g-dev
```

### Boost

Download the `.tar.bz2` or `.tar.gz` from [here](https://www.boost.org/users/download/#live).

Unpack and go into the directory (suppose Boost version is 1.74):

```
$ tar xzf boost_1_74_0.tar.gz
$ cd boost_1_74_0
```

Run `bootstrap.sh` to generate `b2`:

```
$ ./bootstrap.sh
```

Build and install:

```
$ sudo ./b2 --with-system --with-date_time variant=debug link=static threading=multi install
```

**Notes:**

- Only build the specified libraries. `Asio` itself is header only so doesn’t have to be built.
- Only build static libraries (`link=static`)
- If you want to build release version libraries, set `variant=release`. The `debug` and `release` libraries have exactly the same name, so you cannot build them both at the same time.
- Don’t forget the `sudo` since the install prefix is `/usr/local`.

To clean the build, run `b2` with target "clean":

```
$ ./b2 clean
```

The libraries are installed to `/usr/local/lib`. E.g.,

```
$ ls -l /usr/local/lib/libboost*
-rw-r--r--  1 adam  admin   540288 Apr 21 11:01 /usr/local/lib/libboost_date_time.a
...
```

The headers are installed to `/usr/local/include/boost`.

### Googletest

```
$ sudo apt install libgtest-dev
```

### Webcc

Create a build folder under the webcc root (or any other) directory, and `cd` to it:

```
$ mkdir build
$ cd build
```

Generate Makefiles with the following command:

```
$ cmake -G"Unix Makefiles" \
    -DWEBCC_BUILD_UNITTEST=OFF \
    -DWEBCC_BUILD_AUTOTEST=OFF \
    -DWEBCC_BUILD_EXAMPLES=ON \
    -DWEBCC_BUILD_QT_EXAMPLES=OFF \
    -DWEBCC_LOG_LEVEL=0 \
    -DWEBCC_ENABLE_GZIP=1 \
    ..
```

_NOTE: You can create a script (e.g., `gen.sh`) with the above command to avoid typing again and again whenever you want to change an option._

Feel free to change the build options according to your need.

If everything is OK, you can then build with `make`:

```
$ make
```

## Build on Windows

Based on [Visual Studio 2019 Community](https://visualstudio.microsoft.com/vs/community/).

### CMake

Download the latest CMake from https://cmake.org/ and install it.

### Boost

Download the `.7z` or `.zip` from [here](https://www.boost.org/users/download/#live). Unpack it.

Open `x64 Native Tools Command Prompt for VS 2019` from Windows start menu (suppose you are only interested in a x64 build).

In the prompt, `cd` to the Boost root directory. Run `bootstrap.bat` to generate `b2`:

Run `b2` to start the build:

```
$ b2 --with-date_time variant=debug variant=release link=static threading=multi address-model=64 stage
```

Given `address-model=64`, `b2` will not build any x86 libraries.

If you have installed multiple versions of Visual Studio, you'd better add `toolset` option to `b2`, e.g., `toolset=msvc-142` for using Visual Studio 2019.

As you can see, we only need to build `date_time`. Asio itself is a header-only library.

**NOTE:** Even `date_time` is not needed any more since Boost 1.79.

We don't install Boost to any other place (e.g., `C:\Boost`), just `stage` it where it is.

In order for CMake to find Boost, please add an environment variable named `Boost_ROOT` pointing to the root directory of Boost.

### OpenSSL

Click [here](http://slproweb.com/products/Win32OpenSSL.html) to download the OpenSSL installer `Win64 OpenSSL v1.1.1g` (the suffix "g" might change according to revision).

During the installation, you will be asked to copy OpenSSL DLLs (`libcrypto-1_1-x64.dll` and `libssl-1_1-x64.dll`) to "The Windows system directory" or "The OpenSSL libraries (/bin) directory". If you choose the later, remember to add the path (e.g., `C:\Program Files\OpenSSL-Win64\bin`) to the `PATH` environment variable.

![OpenSSL Installation](screenshots/win_openssl_install.png)

OpenSSL can also be statically linked (see `C:\Program Files\OpenSSL-Win64\lib\VC\static`), but it's not recommended. Because the static libraries might not match the version of your Visual Studio.

The only drawback of dynamic link is that you must distribute the OpenSSL DLLs together with your program.

### Zlib

Download Zlib from https://www.zlib.net/.

Use CMake to generate the Visual Studio solution. Click _**Configure**_ button.

By default, `CMAKE_INSTALL_PREFIX` points to a folder like `C:/Program Files (x86)/zlib` which is not what we want.

Change `CMAKE_INSTALL_PREFIX` to a folder where you would like to install all the third party libraries. E.g., `D:/lib/cmake_install_2019_64` (Note: you must use "/" instead of "\\" as the path seperator!).

Remove all the `INSTALL_XXX_DIR` entries. Click _**Configure**_ button again. Now the `INSTALL_XXX_DIR` entries point to the folder defined by `CMAKE_INSTALL_PREFIX`.

Leave all other options untouched, click _**Generate**_ button to generate the Visual Studio solution.

Launch the Visual Studio solution and build the `INSTALL` project for both Debug and Release configurations.

Zlib should now have been installed to the given folder.

In order for CMake to find Zlib during the configuration of Webcc, please add an environment variable named `CMAKE_PREFIX_PATH` which points to the CMake install directory.

### Googletest

Download the latest release of [Googletest](https://github.com/google/googletest/releases).

Use CMake to generate the Visual Studio solution:

![Googletest Installation](screenshots/win_cmake_config_gtest.png)

Please note the highlighted configurations.

The `CMAKE_INSTALL_PREFIX` has been changed to `D:/lib/cmake_install_2019_64` (Note: please use "/" instead of "\\" as the path seperator!). This path should be added to an environment variable named `CMAKE_PREFIX_PATH`.

![CMAKE_PREFIX_PATH](screenshots/win_cmake_prefix_path.png)

After build Googletest in Visual Studio, install it by building `INSTALL` project from the whole solution.

### Webcc

Open CMake, set **Where is the source code** to Webcc root directory (e.g., `D:/github/webcc`), set **Where to build the binaries** to any directory (e.g., `D:/github/webcc/build_2019_64`).

Check _**Grouped**_ and _**Advanced**_ two check boxes.

Click _**Configure**_ button, select the generator and platform (e.g., `x64`) from the popup dialog.

In the center of CMake, you can see a lot of configure options which are grouped. Change them according to your need. E.g., set `WEBCC_ENABLE_GZIP` to `1` to enable the gzip compression.

Click _**Configure**_ button again. If everything is OK, click _**Generate**_ button to generate the Visual Studio solution.

Click _**Open Project**_ button to open Visual Studio and you're ready to build.
