# Build on Linux

_For Ubuntu based Linux distributions._

_NOTE: The usage of C++17 `filesystem` library requires GCC to be version 8 and above. It means that Webcc will not compile on Ubuntu LTS 18.04._

## Install Build Essential

```
sudo apt install build-essential
```

## Install CMake

```
sudo apt install cmake
```

## Install Zlib & OpenSSL

```
sudo apt install zlib1g-dev libssl-dev
```

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
    -DWEBCC_ENABLE_GZIP=1 \g
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
