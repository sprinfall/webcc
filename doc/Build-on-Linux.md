# Build on Linux

_For any Ubuntu based Linux distributions._

## Install Zlib & OpenSSL

```
sudo apt install zlib1g-dev libssl-dev
```

## Install Boost

Download the .tar.bz2 or .tar.gz from [here](https://www.boost.org/users/download/#live). The current version is 1.70.0.

Unpack and go into the directory:

```
tar -xzf boost_1_70_0.tar.bz2
cd boost_1_70_0
```

Run `bootstrap.sh` to generate `b2`:

```
./bootstrap.sh
```

You can change install prefix with `--prefix` (default is `/usr/local`, need `sudo`), but I don't recommend.

Build and install:

```
sudo ./b2 --with-system --with-filesystem --with-date_time variant=debug link=static threading=multi -j4 install
```

Notes:

- Only build the specified libraries. `Asio` itself is header only so doesn’t have to be built.
- Only build static libraries (`link=static`)
- The number after `-j` depends on the number of CPU cores you have.
- If you want to build release version libraries, set `variant=release`. The `debug` and `release` libraries have exactly the same name, so you cannot build them both at the same time.
- Don’t forget the `sudo` since the install prefix is `/usr/local`.

To clean the build, run `b2` with target "clean":

```
./b2 clean
```

The libraries are installed to `/usr/local/lib`. E.g.,

```
$ ls -l /usr/local/lib/libboost*
-rw-r--r--  1 adam  admin   540288 Apr 21 11:01 /usr/local/lib/libboost_date_time.a
-rw-r--r--  1 adam  admin  1717736 Apr 21 11:01 /usr/local/lib/libboost_filesystem.a
-rw-r--r--  1 root  admin     2976 Apr 21 11:01 /usr/local/lib/libboost_system.a
```

The headers are installed to `/usr/local/include/boost`.

## Build Webcc

Create a build folder under the root (or any other) directory, and `cd` to it:

```
mkdir build
cd build
```

Generate Makefiles with the following command:

```
cmake -G"Unix Makefiles" \
    -DCMAKE_INSTALL_PREFIX=~ \
    -DWEBCC_ENABLE_LOG=1 \
    -DWEBCC_LOG_LEVEL=0 \
    -DWEBCC_ENABLE_SSL=1 \
    -DWEBCC_ENABLE_GZIP=1 \
    -DWEBCC_ENABLE_AUTOTEST=OFF \
    -DWEBCC_ENABLE_UNITTEST=OFF \
    -DWEBCC_ENABLE_EXAMPLES=OFF \
    ..
```

_NOTE: You can create a script (e.g., `gen.sh`) with the above command to avoid typing again and again whenever you want to change an option._

Feel free to change the build options according to your need.

CMake variable `CMAKE_INSTALL_PREFIX` defines where to install the output library and header files. The default is `/usr/local`. But this feature needs rework. I should come back to this later.

If everything is OK, you can then build with `make`:

```
make -j4
```

The number after `-j` depends on how many CPU cores you have. You can just ignore `-j` option.

Install the libraries:

```bash
make install
```

If you `WEBCC_ENABLE_AUTOTEST` was `ON`, you can run the automation test:

```
cd autotest
./webcc_autotest
```