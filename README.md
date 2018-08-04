# webcc

A lightweight C++ REST and SOAP client and server library based on *Boost.Asio*.

Please turn to our [Wiki](https://github.com/sprinfall/webcc/wiki) for more tutorials and guides.

## Quick Start

### REST Server

Suppose you want to create a book server, and provide the following operations with RESTful API:

- Query books based on some criterias.
- Add a new book.
- Get the detailed information of a book.
- Update the information of a book.
- Delete a book.

The first two operations can be implemented by deriving from `webcc::RestListService`:

```cpp
class BookListService : public webcc::RestListService {
 protected:
  // Query books based on some criterias.
  // GET /books?<query>
  bool Get(const webcc::UrlQuery& query,
           std::string* response_content) override;

  // Add a new book.
  // POST /books
  // The new book's data is attached as request content in JSON format.
  bool Post(const std::string& request_content,
            std::string* response_content) override;
};
```

The others, derive from `webcc::RestDetailService`:

```cpp
// The URL is like '/book/{BookID}', and the 'url_sub_matches' parameter
// contains the matched book ID.
class BookDetailService : public webcc::RestDetailService {
 protected:
  // Get the detailed information of a book.
  bool Get(const std::vector<std::string>& url_sub_matches,
           const webcc::UrlQuery& query,
           std::string* response_content) override;

  // Update the information of a book.
  bool Put(const std::vector<std::string>& url_sub_matches,
           const std::string& request_content,
           std::string* response_content) override;

  // Delete a book.
  bool Delete(const std::vector<std::string>& url_sub_matches) override;
};
```

As you can see, all you have to do is to override the proper virtual functions which are named after HTTP methods.

The detailed implementation is out of the scope of this document, but here is an example:

```cpp
bool BookDetailService::Get(const std::vector<std::string>& url_sub_matches,
                            const webcc::UrlQuery& query,
                            std::string* response_content) {
  if (url_sub_matches.size() != 1) {
    return false;
  }
  const std::string& book_id = url_sub_matches[0];

  // Get the book by ID from database.
  // Convert the book details to JSON string.
  // Assign JSON string to response_content.
}
```

Last step, bind the services and run the server:

```cpp
webcc::RestServer server(8080, 2);

server.Bind(std::make_shared<BookListService>(), "/books", false);
server.Bind(std::make_shared<BookDetailService>(), "/book/(\\d+)", true);

server.Run();
```

**Please see the `example/rest` folder for the complete examples (including the client).**

## Build Instructions

A lot of C++11 features are used, e.g., `std::move`. But C++14 is not required.
(It means that you can still build `webcc` using VS2013 on Windows.)

[CMake 3.1.0+](https://cmake.org/) is required as the build system. But if you don't use CMake, you can just copy the `src/webcc` folder to your own project then manage it by yourself.

[C++ Boost](https://www.boost.org/) should be 1.66+ because Asio made some broken changes to the API in 1.66.

### Build Options

The following CMake options determine how you build the projects. They are quite self-explanatory.

```cmake
option(WEBCC_ENABLE_LOG "Enable logging?" ON)
option(WEBCC_ENABLE_SOAP "Enable SOAP support (need pugixml)?" ON)
option(WEBCC_BUILD_UNITTEST "Build unit test?" ON)
option(WEBCC_BUILD_EXAMPLE "Build examples?" ON)

set(WEBCC_LOG_LEVEL "VERB" CACHE STRING "Log level (VERB, INFO, WARN, ERRO or FATA)")
```

Options `WEBCC_ENABLE_LOG` and `WEBCC_LOG_LEVEL` together define how logging behaves. See (Wiki/Logging)[https://github.com/sprinfall/webcc/wiki/Logging] for more details.

If `WEBCC_ENABLE_SOAP` is `ON`, **pugixml** (already included) is used to parse and compose XML strings.

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
    -DWEBCC_ENABLE_LOG=ON \
    -DWEBCC_LOG_LEVEL=VERB \
    -DWEBCC_ENABLE_SOAP=ON \
    -DWEBCC_BUILD_UNITTEST=OFF \
    -DWEBCC_BUILD_EXAMPLE=ON \
    ..
```
Feel free to change the build options (`ON` or `OFF`).
CMake variable `CMAKE_INSTALL_PREFIX` defines where to install the output library and header files. The default is `/usr/local`.

If everything is OK, then you can build with `make`:
```bash
$ make -j4  # or -j8, depending on how many CPU cores you have.
```

Then install:
```bash
$ make install
```
