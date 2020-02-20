# Build on Windows

VS 2013 and above is required for building `Webcc` on Windows.

## Install Boost

Download the .7z or .zip from [here](https://www.boost.org/users/download/#live). Unpack it.

Open `VS Native Tools Command Prompt` from Windows start menu.

![VS Cmd Prompts Win7](screenshots/vs_cmd_prompts_win7.png)

E.g., I choose `VS2015 x64 Native Tools Command Prompt` to build a 64-bit Boost.

In the prompt, `cd` to the Boost root directory. Run `bootstrap.bat` to generate `b2.exe`:

Run `b2.exe` to start the build:

```
b2 --with-system --with-filesystem --with-date_time variant=debug variant=release link=static threading=multi stage
```

Only `system`, `filesystem` and `date_time` are built. `Asio` is a header-only library.

We don't install Boost to another place (e.g., `C:\Boost`) by specifying `stage` instead of `install` at the end of the command. In order to let CMake find Boost, please add an environment variable named `Boost_ROOT` pointing to the root directory of Boost.

## Install OpenSSL

Download from [here](http://slproweb.com/products/Win32OpenSSL.html).

The following installers are recommended for development:

- Win64 OpenSSL v1.1.0k
- Win32 OpenSSL v1.1.0k

During the installation, you will be asked to copy OpenSSL DLLs (`libcrypto-1_1-x64.dll` and `libssl-1_1-x64.dll`) to "The Windows system directory" or "The OpenSSL libraries (/bin) directory". If you choose the later, remember to add the path (e.g., `C:\OpenSSL-Win64\bin`) to the `PATH` environment variable.

![OpenSSL Installation](screenshots/win_openssl_install.png)

OpenSSL can also be statically linked (see `C:\OpenSSL-Win64\lib\VC\static`), but it's not recommended. Because the static libraries might not match the version of your VS.

The only drawback of dynamic link is that you must distribute the OpenSSL DLLs together with your program.

## Install Zlib

Zlib has been included in `third_party\src`. Maybe it's not a good idea.

In order to integrate `webcc` into your project, you have to integrate this zlib, too. This makes it complicated. I will come back to this later.

## Generate VS Solution

Open CMake, set **Where is the source code** to Webcc root directory (e.g., `D:/github/webcc`), set **Where to build the binaries** to any directory (e.g., `D:/github/webcc/build_vs2015_64`).

Check _**Grouped**_ and _**Advanced**_ two check boxes.

Click _**Configure**_ button, select the generator and platform (win32 or x64) from the popup dialog.

![CMake generator](screenshots/win_cmake_generator.png)

In the center of CMake, you can see a lot of configure options which are grouped. Change them according to your need. E.g., set `WEBCC_ENABLE_SSL` to `1` to enable OpenSSL.

![CMake config](screenshots/win_cmake_config.png)

Click _**Configure**_ button again. OpenSSL should be found.

![CMake config OpenSSL](screenshots/win_cmake_config_openssl.png)

Click _**Configure**_ button again. If everything is OK, click _**Generate**_ button to generate the VS solution.

Click _**Open Project**_ button to open VS.