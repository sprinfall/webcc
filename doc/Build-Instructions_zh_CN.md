# 编译指南

* [Overview](#overview)
* [Build on Ubuntu](#build-on-ubuntu)
* [Build on Windows](#build-on-windows)

## Overview

### 编译依赖

* [Boost 1.66+](https://www.boost.org/)（asio, system, date_time）
* [OpenSSL](https://www.openssl.org/) （为了支持 HTTPS）
* [Zlib](https://www.zlib.net/) （为了支持 GZIP 压缩，可选）
* [Googletest](https://github.com/google/googletest)（为了自动化测试和单元测试，可选）
* [CMake](https://cmake.org/)

Zlib 是 **可选的**，因为 GZIP 压缩可以被禁掉，详见下面的编译选项说明。

Googletest 也是 **可选的**，除非你想要编译自动化测试和单元测试。

### 编译选项

下面这些 CMake 选项决定了你将如何编译 webcc 的各个项目，它们应该都不难理解。

```cmake
option(WEBCC_BUILD_UNITTEST "是否编译单元测试？" OFF)
option(WEBCC_BUILD_AUTOTEST "是否编译自动化测试？" OFF)
option(WEBCC_BUILD_EXAMPLES "是否编译示例？" OFF)
option(WEBCC_BUILD_QT_EXAMPLES "是否编译 Qt 程序示例？" OFF)

set(WEBCC_LOG_LEVEL 2 CACHE STRING "日志等级（0: VERB, 1: INFO, 2: USER, 3: WARN, 4:ERRO, 5:NONE）")
set(WEBCC_ENABLE_GZIP 0 CACHE STRING "是否开启 GZIP 压缩？（1：是，0：否）")
```

其中，`option()` 定义的选项取 `ON` 或 `OFF`，相当于布尔值；`set()` 定义的选项通常取 0 或 1 表示开关，也相当于布尔值；`WEBCC_LOG_LEVEL` 取 0 ~ 5，其中 0 ~ 4 代表五个不同的日志等级，5 则表示完全地禁用日志。

此外，自动化测试调用的是真正的 HTTP 服务器——[httpbin.org](http://httpbin.org/)（虽然它有时候可能无法访问）。

### 集成

虽然这里介绍了详细的编译过程，但是真正把 webcc 集成到你的项目中却是另一回事。

因为 webcc 暂时还不支持 CMake Install，所以，无法像安装 OpenSSL 或 Zlib 等第三方库那样来安装 webcc，webcc 也就无法自动被 CMake “找到”。

目前，最推荐的集成方式仍然是「源码集成」，也就是把 webcc 整个子目录拷贝到你自己的项目中，并添加相应的 CMake 配置。具体怎么做，这里有一个示例项目可供参考：[https://github.com/sprinfall/webcc-integration](https://github.com/sprinfall/webcc-integration)。

## Build on Ubuntu

*注意：基于 Ubuntu 18.04 LTS*

如果还没有安装过 `build-essential`（包含 C++ 编译器等），请先执行下面这条命令：

```
$ sudo apt install build-essential
```

### CMake

拿 Ubuntu 18.04 来说，直接通过 apt 安装的 CMake 版本仅为 3.10，并不能满足我们的需求。Boost 1.66 就需要 CMake 3.11 以上，更不用说 Boost 1.74 了。

请参考 [https://apt.kitware.com/](https://apt.kitware.com/) 来安装较新版本的 CMake。

### OpenSSL

```
$ sudo apt install libssl-dev
```

### Zlib

```
$ sudo apt install zlib1g-dev
```

### Boost

下面以 Boost 1.74 为例。

从 Boost 官网下载 [源码包](https://www.boost.org/users/download/#live)，解压并进入根目录：

```
$ tar xzf boost_1_74_0.tar.gz
$ cd boost_1_74_0
```

运行 `bootstrap.sh` 以生成 Boost 的编译工具 `b2`：

```
$ ./bootstrap.sh
```

编译并安装：

```
$ sudo ./b2 --with-system --with-date_time variant=debug link=static threading=multi install
```

**注意：**

- 只编译指定的库（节省时间），Asio 本身只有头文件，不需要编译；
- 只编译静态库（`link=static`）；
- 如果你想编译 release 版的库，设置 `variant=release`；
- 最后，不要忘记 `sudo`，因为缺省的安装目录为 `/usr/local`。

如果想清理编译产生的临时文件，请执行：

```
$ ./b2 clean
```

库文件被安装到了 `/usr/local/lib`，比如：

```
$ ls -l /usr/local/lib/libboost*
-rw-r--r--  1 adam  admin   540288 Apr 21 11:01 /usr/local/lib/libboost_date_time.a
...
```

头文件则被安装到了：`/usr/local/include/boost`。

### Googletest

```
$ sudo apt install libgtest-dev
```

### Webcc

在 webcc 的根目录下创建一个子目录（也可以指定其他任何一个位置），专门用来编译：

```
$ mkdir build
$ cd build
```

通过下面的 cmake 命令生成 Makefiles：

```
$ cmake -G"Unix Makefiles" \
    -DBUILD_UNITTEST=OFF \
    -DBUILD_AUTOTEST=OFF \
    -DBUILD_EXAMPLES=ON \
    -DBUILD_QT_EXAMPLES=OFF \
    -DWEBCC_LOG_LEVEL=0 \
    -DWEBCC_ENABLE_GZIP=1 \
    ..
```

*注意：你可以把上面这个多行的命令保存为一个脚本（比如就叫 `gen.sh`），这样方便后续修改和输入。*

请根据需要修改上面这些选项的值。其中，AUTOTEST 和 UNITTEST 均依赖于 Googletest。

最后，执行 `make` 即可（`-j4` 指用4个线程并行编译，请根据你的 CPU 有几个核来设置，也可以直接忽略该选项）：

```
$ make -j4
```

## Build on Windows

下面以 [Visual Studio 2019 Community](https://visualstudio.microsoft.com/vs/community/) 为例。

### CMake

直接从 [CMake 官网](https://cmake.org/) 下载安装最新版即可。

### Boost

下面以 Boost 1.74 为例，从 Boost 官网下载 [源码包](https://www.boost.org/users/download/#live)，解压。

从 Windows 开始菜单打开 `x64 Native Tools Command Prompt for VS 2019`（假定你只考虑 64 位编译）。

在这个命令行提示里，`cd` 到刚刚解压的 Boost 根目录，并运行 `bootstrap.bat` 来生成 Boost 的编译工具 `b2`。

然后，使用 `b2` 进行编译：

```
$ b2 --with-system --with-date_time variant=debug variant=release link=static threading=multi address-model=64 stage
```

*注意：指定 `address-model=64` 的话，`b2` 就不再编译 32 (x86) 位版本的库了，这样可以节省时间。*

如你所见，我们只编译 `system` 和 `date_time`，Asio 本身只有头文件，不需要编译。

我们也不安装 Boost 到某个其他目录（比如缺省的 `C:\Boost`），只是简单的在原地 `stage` 。

为了让 CMake 能够找到 Boost，请添加一个环境变量 `Boost_ROOT`，让它指向 Boost 的根目录。

### OpenSSL

从 [这里](http://slproweb.com/products/Win32OpenSSL.html) 下载 OpenSSL 安装包 `Win64 OpenSSL v1.1.1g`（注意：后缀 "g" 随版本更新会有变化）。

在 OpenSSL 安装过程中，它会问你，把 OpenSSL DLL (`libcrypto-1_1-x64.dll`, `libssl-1_1-x64.dll`) 拷贝到 "The Windows system directory" 还是 "The OpenSSL libraries (/bin) directory"？如果你选择了后者，记得把 `C:\Program Files\OpenSSL-Win64\bin` 加到 `PATH` 环境变量，否则程序运行时找不到 OpenSSL DLL。

![OpenSSL Installation](screenshots/win_openssl_install.png)

OpenSSL 也可以静态链接（见 `C:\Program Files\OpenSSL-Win64\lib\VC\static`），但不推荐这么做，因为这些静态库所使用的 Visual Studio 与你正在用的版本不一定兼容。

使用动态链接的唯一缺点是，在你发布你的程序时，得把这些 OpenSSL DLL 一起打包，并且安装到与 EXE 同一个目录。

### Zlib

从 [这里](https://www.zlib.net/) 下载 Zlib。

使用 CMake 生成 Visual Studio 解决方案。点击 **Configure** 按钮。

缺省情况下，`CMAKE_INSTALL_PREFIX` 变量指向了一个这样的目录：`C:/Program Files (x86)/zlib`，很明显这不是我们想要的。

请把 `CMAKE_INSTALL_PREFIX` 改成一个专门用于安装第三方库的目录，比如 `D:/lib/cmake_install_2019_64`。注意，在指定这个路径时，必须使用 `/` 而不是 `\\`，否则 CMake 无法使用安装的库。

接下来，请手动删掉所有形如 `INSTALL_XXX_DIR` 的条目。

再一次点击 **Configure** 按钮，现在，`INSTALL_XXX_DIR` 都改为指向 `CMAKE_INSTALL_PREFIX` 所指定的目录了。

其他选项一律不做修改。点击 **Generate** 按钮，生成 Visual Studio 解决方案。

打开这个 Visual Studio 解决方案，直接编译 `INSTALL` 这个项目，Debug 和 Release 都要编译。

然后，Zlib 应该就已经被安装到刚刚指定的那个目录了。

为了让 CMake 能够找到 Zlib（以及其他所有通过 CMake 安装到那个目录的库），我们需要添加一个环境变量叫 `CMAKE_PREFIX_PATH`，让它指向上面指定的这个 CMake 安装目录（即 `D:/lib/cmake_install_2019_64`，注意路径不要用 `\\`）。

### Googletest

从 GitHub 下载最新发布版本：[Googletest](https://github.com/google/googletest/releases)

使用 CMake 生成 Visual Studio 解决方案：

![Googletest Installation](screenshots/win_cmake_config_gtest.png)

请注意被高亮的配置。

同时，`CMAKE_INSTALL_PREFIX` 被改成了 `D:/lib/cmake_install_2019_64`。具体的安装方式跟 Zlib 一节描述的一模一样，这里就不再赘述了。

![CMAKE_PREFIX_PATH](screenshots/win_cmake_prefix_path.png)

### Webcc

打开 CMake，设置 **Where is the source code** 到 webcc 的根目录，比如 `D:/github/webcc`，设置 **Where to build the binaries** 到任一目录，比如：`D:/github/webcc/build_2019_64`。

在 CMake 界面上勾选 **Grouped** 和 **Advanced** 这两个选项，以便更方便地查看 CMake 的选项。

点击 **Configure** 按钮，在弹出的对话框中选择恰当的 generator 和 platform (比如 `x64`) 。

在 CMake 界面的中间区域，你可以看到很多分了组的配置选项，请根据需要修改相应的选项，比如，设置 `WEBCC_ENABLE_GZIP` 为 `1` 就可以启用 gzip 压缩。

再次点击 **Configure** 按钮，如果一切 OK，点击 **Generate** 按钮，生成 Visual Studio 解决方案。

打开 Visual Studio 解决方案，就可以随意编译了！
