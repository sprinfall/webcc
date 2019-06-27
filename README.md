# webcc

C++ client and server library for HTTP and REST based on [Boost.Asio](https://www.boost.org/doc/libs/release/libs/asio/).

Please turn to our [Wiki](https://github.com/sprinfall/webcc/wiki) (under construction) for more tutorials and guides.

Wondering how to build Webcc? Check [Build Instructions](https://github.com/sprinfall/webcc/wiki/Build-Instructions).

Git repo: https://github.com/sprinfall/webcc. Please check this one instead of the forked for the latest features.

**Features**

- Cross-platform: Linux, Windows and Mac
- Easy-to-use client API inspired by Python [requests](https://2.python-requests.org//en/master/)
- SSL/HTTPS support with OpenSSL (optional)
- GZip compression support with Zlib (optional)
- Persistent connections (Keep-Alive)
- Basic & Token authorization
- Timeout control
- Source code follows [Google C++ Style](https://google.github.io/styleguide/cppguide.html)
- Automation tests and unit tests included
- No memory leak detected by [VLD](https://kinddragon.github.io/vld/)
- etc.

## Client API Examples

A complete client example: 
```cpp
#include <iostream>

#include "webcc/client_session.h"
#include "webcc/logger.h"

int main() {
  // Configure logger.
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);
  
  // Session provides convenient request APIs, stores request configurations
  // and manages persistent connenction.
  webcc::ClientSession session;

  // Catch exceptions for error handling.
  try {
    // Send a HTTP GET request.
    auto r = session.Get("http://httpbin.org/get");

    // Print the response content data.
    std::cout << r->content() << std::endl;

  } catch (const webcc::Error& error) {
    std::cout << error << std::endl;
  }

  return 0;
}
```

The `Get()` method is nothing but a shortcut of `Request()`. Using `Request()` directly is more complicated:
```cpp
auto r = session.Request(webcc::RequestBuilder{}.Get().
                         Url("http://httpbin.org/get")
                         ());
```
As you can see, a helper class named `RequestBuilder` is used to chain the parameters and finally build (don't miss the `()` operator) a request object.

Both the shortcut and `Request()` accept URL query parameters:

```cpp
// Query parameters are passed using a std::vector. 
session.Get("http://httpbin.org/get", { "key1", "value1", "key2", "value2" });

session.Request(webcc::RequestBuilder{}.Get().
                Url("http://httpbin.org/get").
                Query("key1", "value1").
                Query("key2", "value2")
                ());
```

Adding additional headers is also easy:
```cpp
session.Get("http://httpbin.org/get",
            {"key1", "value1", "key2", "value2"},
            {"Accept", "application/json"});  // Also a std::vector
                
session.Request(webcc::RequestBuilder{}.Get().
                Url("http://httpbin.org/get").
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
You can then parse `r->content()` to JSON object with your favorite JSON library. My choice for the examples is [jsoncpp](https://github.com/open-source-parsers/jsoncpp). But the library itself doesn't understand JSON nor require one. It's up to you to choose the most appropriate JSON library.

## Server API Examples

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
      return Get(request->query());
    }

    if (request->method() == "POST") {
      return Post(request);
    }

    return {};
  }
  
private:
  // Get a list of books based on query parameters.
  webcc::ResponsePtr Get(const webcc::UrlQuery& query);

  // Create a new book.
  // The new book's data is attached as request content in JSON format.
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
      return Get(request->args(), request->query());
    }

    if (request->method() == "PUT") {
      return Put(request, request->args());
    }

    if (request->method() == "DELETE") {
      return Delete(request->args());
    }

    return {};
  }
  
protected:
  // Get the detailed information of a book.
  webcc::ResponsePtr Get(const webcc::UrlArgs& args,
                         const webcc::UrlQuery& query);

  // Update a book.
  webcc::ResponsePtr Put(webcc::RequestPtr request,
                         const webcc::UrlArgs& args);

  // Delete a book.
  webcc::ResponsePtr Delete(const webcc::UrlArgs& args);
};
```

The detailed implementation is out of the scope of this README, but here is an example:

```cpp
webcc::ResponsePtr BookDetailView::Get(const webcc::UrlArgs& args,
                                       const webcc::UrlQuery& query) {
  if (args.size() != 1) {
    // Using kNotFound means the resource specified by the URL cannot be found.
    // kBadRequest could be another choice.
    return webcc::ResponseBuilder{}.NotFound()();
  }

  const std::string& book_id = args[0];

  // Get the book by ID from, e.g., the database.
  // ...

  if (<NotFound>) {
    // There's no such book with the given ID. 
    return webcc::ResponseBuilder{}.NotFound()();
  } else {
    // Convert the book to JSON string and set as response content.
    return webcc::ResponseBuilder{}.OK().Data(<JsonStringOfTheBook>).Json()();
  }
}
```

Last step, route URLs to the proper views and run the server:

```cpp
webcc::Server server(8080, 2);

server.Route("/books", std::make_shared<BookListView>(), { "GET", "POST" });

server.Route(webcc::R("/books/(\\d+)"), std::make_shared<BookDetailView>(),
             { "GET", "PUT", "DELETE" });

server.Run();
```

Please see [examples/rest_book_server.cc](https://github.com/sprinfall/webcc/tree/master/examples/rest_book_server.cc) for more details.
