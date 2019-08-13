# webcc

[同学们，【中文版】在这里！](README_zh_CN.md)

Lightweight C++ HTTP client and server library based on [Boost Asio](https://www.boost.org/doc/libs/release/libs/asio/).

Please turn to our [Wiki](https://github.com/sprinfall/webcc/wiki) (under construction) for more tutorials and guides.

Wondering how to build Webcc? Check [Build Instructions](https://github.com/sprinfall/webcc/wiki/Build-Instructions).

Git repo: https://github.com/sprinfall/webcc. Please check this one instead of the forked for the latest features.

**Features**

- Cross-platform: Linux, Windows and Mac
- Easy-to-use client API inspired by Python [requests](https://2.python-requests.org//en/master/)
- SSL/HTTPS support with OpenSSL (optional)
- GZip compression support with Zlib (optional)
- Persistent (Keep-Alive) connections
- Data streaming
    - for uploading and downloading large files on client
    - for serving and receiving large files on server
- Basic & Token authorization
- Timeout control
- Source code follows [Google C++ Style](https://google.github.io/styleguide/cppguide.html)
- Automation tests and unit tests included
- No memory leak detected by [VLD](https://kinddragon.github.io/vld/)
- etc.

## Client API

Let's start from a complete client example:

```cpp
#include <iostream>

#include "webcc/client_session.h"
#include "webcc/logger.h"

int main() {
  // Configure logger to print only to console.
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);
  
  // Session provides convenient request APIs, stores request configurations
  // and manages persistent connenctions.
  webcc::ClientSession session;

  // Catch exceptions for error handling.
  try {
    // Send a HTTP GET request.
    auto r = session.Get("http://httpbin.org/get");

    // Print the response data.
    std::cout << r->data() << std::endl;

  } catch (const webcc::Error& error) {
    std::cerr << error << std::endl;
  }

  return 0;
}
```

The `Get()` method is nothing but a shortcut of `Request()`. Using `Request()` directly is more complicated:

```cpp
auto r = session.Request(webcc::RequestBuilder{}.Get("http://httpbin.org/get")());
```

As you can see, a helper class named `RequestBuilder` is used to chain the parameters and finally build a request object. Please pay attention to the `()` operator.

Both the shortcut and `Request()` accept URL query parameters:

```cpp
// Query parameters are passed using a std::vector. 
session.Get("http://httpbin.org/get", { "key1", "value1", "key2", "value2" });

session.Request(webcc::RequestBuilder{}.
                Get("http://httpbin.org/get").
                Query("key1", "value1").
                Query("key2", "value2")
                ());
```

Adding additional headers is also easy:

```cpp
session.Get("http://httpbin.org/get",
            {"key1", "value1", "key2", "value2"},
            {"Accept", "application/json"});  // Also a std::vector
                
session.Request(webcc::RequestBuilder{}.
                Get("http://httpbin.org/get").
                Query("key1", "value1").
                Query("key2", "value2").
                Header("Accept", "application/json")
                ());
```

Accessing HTTPS has no difference from HTTP:

```cpp
session.Get("https://httpbin.org/get");
```

*NOTE: The HTTPS/SSL support requires the build option `WEBCC_ENABLE_SSL` to be enabled.*

Listing GitHub public events is not a big deal:

```cpp
auto r = session.Get("https://api.github.com/events");
```

You can then parse `r->data()` to JSON object with your favorite JSON library. My choice for the examples is [jsoncpp](https://github.com/open-source-parsers/jsoncpp). But Webcc itself doesn't understand JSON nor require one. It's up to you to choose the most appropriate JSON library.

The shortcuts (`Get()`, `Post()`, etc.) are easier to use but `RequestBuilder` is more powerful. It provides a lot of functions for you to customize the request. Let's see more examples.

In order to list the followers of an authorized GitHub user, you need either **Basic Authorization**:

```cpp
session.Request(webcc::RequestBuilder{}.
                Get("https://api.github.com/user/followers").
                AuthBasic(login, password)  // Should be replaced by real login and password
                ());
```

Or **Token Authorization**:

```cpp
session.Request(webcc::RequestBuilder{}.
                Get("https://api.github.com/user/followers").
                AuthToken(token)  // Should be replaced by a valid token
                ());
```

Though **Keep-Alive** (i.e., persistent connection) is a good feature and enabled by default, you can turn it off:

```cpp
auto r = session.Request(webcc::RequestBuilder{}.
                         Get("http://httpbin.org/get").
                         KeepAlive(false)  // No Keep-Alive
                         ());
```

The API for other HTTP requests is no different from GET.

POST request needs a body which is normally a JSON string for REST API. Let's post a small UTF-8 encoded JSON string:

```cpp
session.Request(webcc::RequestBuilder{}.
                Post("http://httpbin.org/post").
                Body("{'name'='Adam', 'age'=20}").
                Json().Utf8()
                ());
```

Webcc has the ability to stream large response data to a file. This is especially useful when downloading files.

```cpp
auto r = session.Request(webcc::RequestBuilder{}.
                         Get("http://httpbin.org/image/jpeg")(),
                         true);  // !!!

// Move the streamed file to your destination.
r->file_body()->Move("./wolf.jpeg");
```

Please check the [examples](https://github.com/sprinfall/webcc/tree/master/examples/) for more information. 

## Server API

### Hello, World!

```cpp
class HelloView : public webcc::View {
public:
  webcc::ResponsePtr Handle(webcc::RequestPtr request) override {
    if (request->method() == "GET") {
      return webcc::ResponseBuilder{}.OK().Body("Hello, World!")();
    }

    return {};
  }
};

int main() {
  try {
    webcc::Server server(8080);

    server.Route("/", std::make_shared<HelloView>());

    server.Run();

  } catch (const std::exception&) {
    return 1;
  }

  return 0;
}
```

### Book Store

Suppose you want to create a book server and provide the following operations with RESTful API:

- Query books based on some criterias.
- Add a new book.
- Get the detailed information of a book.
- Update the information of a book.
- Delete a book.

The first two operations are implemented by `BookListView` deriving from `webcc::View`:

```cpp
class BookListView : public webcc::View {
public:
  webcc::ResponsePtr Handle(webcc::RequestPtr request) override {
    if (request->method() == "GET") {
      return Get(request);
    }

    if (request->method() == "POST") {
      return Post(request);
    }

    return {};
  }
  
private:
  // Get a list of books based on query parameters.
  webcc::ResponsePtr Get(webcc::RequestPtr request);

  // Create a new book.
  // The new book's data is attached as request data in JSON format.
  webcc::ResponsePtr Post(webcc::RequestPtr request);
};
```

Other operations are implemented by `BookDetailView`:

```cpp
// The URL is like '/books/{BookID}', and the 'args' parameter contains
// the matched book ID.
class BookDetailView : public webcc::View {
public:
  webcc::ResponsePtr Handle(webcc::RequestPtr request) override {
    if (request->method() == "GET") {
      return Get(request);
    }

    if (request->method() == "PUT") {
      return Put(request);
    }

    if (request->method() == "DELETE") {
      return Delete(request);
    }

    return {};
  }
  
protected:
  // Get the detailed information of a book.
  webcc::ResponsePtr Get(webcc::RequestPtr request);

  // Update a book.
  webcc::ResponsePtr Put(webcc::RequestPtr request);

  // Delete a book.
  webcc::ResponsePtr Delete(webcc::RequestPtr request);
};
```

The detailed implementation is out of the scope of this README, but here is an example:

```cpp
webcc::ResponsePtr BookDetailView::Get(webcc::RequestPtr request) {
  if (request->args().size() != 1) {
    // NotFound means the resource specified by the URL cannot be found.
    // BadRequest could be another choice.
    return webcc::ResponseBuilder{}.NotFound()();
  }

  const std::string& book_id = request->args()[0];

  // Get the book by ID from, e.g., the database.
  // ...

  if (<NotFound>) {
    // There's no such book with the given ID. 
    return webcc::ResponseBuilder{}.NotFound()();
  }

  // Convert the book to JSON string and set as response data.
  return webcc::ResponseBuilder{}.OK().Data(<JsonStringOfTheBook>).
      Json().Utf8()();
}
```

Last step, route URLs to the proper views and run the server:

```cpp
int main(int argc, char* argv[]) {
  // ...

  try {
    webcc::Server server(8080);

    server.Route("/books",
                 std::make_shared<BookListView>(),
                 { "GET", "POST" });

    server.Route(webcc::R("/books/(\\d+)"),
                 std::make_shared<BookDetailView>(),
                 { "GET", "PUT", "DELETE" });

    server.Run();

  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
```

Please see [examples/rest_book_server.cc](https://github.com/sprinfall/webcc/tree/master/examples/rest_book_server.cc) for more details.
