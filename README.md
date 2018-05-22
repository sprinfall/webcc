# webcc

A lightweight C++ REST and SOAP client and server library based on *Boost.Asio*.

## A Quick Look

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
           std::string* response_content) final;

  // Add a new book.
  // POST /books
  // The new book's data is attached as request content in JSON format.
  bool Post(const std::string& request_content,
            std::string* response_content) final;
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
           std::string* response_content) final;

  // Update the information of a book.
  bool Put(const std::vector<std::string>& url_sub_matches,
           const std::string& request_content,
           std::string* response_content) final;

  // Delete a book.
  bool Delete(const std::vector<std::string>& url_sub_matches) final;
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

Last step, register the services and run the server:

```cpp
webcc::RestServer server(8080, 2);

server.Bind(std::make_shared<BookListService>(), "/books", false);
server.Bind(std::make_shared<BookDetailService>(), "/book/(\\d+)", true);

server.Run();
```

**Please see the `example` folder for the complete examples (including the client).**

## Build Instructions

A lot of C++11 features are used, e.g., `std::move`. But C++14 is not required.
(It means that you can still build `webcc` using VS2013 on Windows.)

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

### Build On Linux

Create a build folder under the root (or any other) directory, and `cd` to it:
```bash
mkdir build
cd build
```
Generate Makefiles with the following command:
```bash
cmake -G"Unix Makefiles" \
    -DWEBCC_ENABLE_LOG=ON \
    -DWEBCC_ENABLE_SOAP=ON \
    -DWEBCC_BUILD_UNITTEST=OFF \
    -DWEBCC_BUILD_REST_EXAMPLE=ON \
    -DWEBCC_BUILD_SOAP_EXAMPLE=OFF \
    ..
```
Feel free to change the build options (`ON` or `OFF`).
Then make:
```bash
$ make -j4
```
