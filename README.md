# Webcc - C++ HTTP Library

**NOTE：**
- **[legacy](https://github.com/sprinfall/webcc/tree/legacy)** branch only uses limited C++11 features, so it could be built by old compilers like _VS2013_ and _GCC 4.8_.

----

[__中文版 README__](README_zh_CN.md)

Lightweight C++ HTTP __client and server__ library based on [Boost Asio](https://www.boost.org/doc/libs/release/libs/asio/) for __embedding__ purpose.

=> [Build Instructions](doc/Build-Instructions.md) ([__中文版__](doc/Build-Instructions_zh_CN.md))

Git repo: https://github.com/sprinfall/webcc. Please check this one instead of the forked for the latest features.

**Contents**
* [Overview](#overview)
* [Client API](#client-api)
    * [A Complete Example](#a-complete-example)
    * [Request Builder](#request-builder)
    * [HTTPS](#https)
    * [Invoking GitHub REST API](#invoking-github-rest-api)
    * [Authorization](#authorization)
    * [Keep-Alive (Persistent Connection)](#keep-alive)
    * [POST Request](#post-request)
    * [Downloading Files](#downloading-files)
    * [Uploading Files](#uploading-files)
    * [Client API and Threads](#client-api-and-threads)
* [Server API](#server-api)
    * [A Minimal Server](#a-minimal-server)
    * [URL Route](#url-route)
    * [Running A Server](#running-a-server)
    * [Response Builder](#response-builder)
    * [REST Book Server](#rest-book-server)
* [IPv6 Support](#ipv6-support)
    * [IPv6 Server](#ipv6-server)
    * [IPv6 Client](#ipv6-client)
    
## Overview

- Cross-platform: Windows, Linux and MacOS
- Easy-to-use client API inspired by Python [requests](https://github.com/psf/requests)
- IPv6 support
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

## Client API

### A Complete Example

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
    auto r = session.Send(webcc::RequestBuilder{}.
                          Get("http://httpbin.org/get")
                          ());

    // Print the response data.
    std::cout << r->data() << std::endl;

  } catch (const webcc::Error& error) {
    std::cerr << error << std::endl;
  }

  return 0;
}
```

### Request Builder

As you can see, a helper class named `RequestBuilder` is used to chain the parameters and finally build a request object. Please pay attention to the `()` operator.

URL query parameters can be easily added through `Query()` method:

```cpp
session.Send(webcc::RequestBuilder{}.
             Get("http://httpbin.org/get").
             Query("key1", "value1").Query("key2", "value2")
             ());
```

Adding additional headers is also easy:

```cpp
session.Send(webcc::RequestBuilder{}.
             Get("http://httpbin.org/get").
             Header("Accept", "application/json")
             ());
```

### HTTPS

Accessing HTTPS has no difference from HTTP:

```cpp
session.Send(webcc::RequestBuilder{}.Get("https://httpbin.org/get")());
```

*NOTE: The HTTPS/SSL support requires the build option `WEBCC_ENABLE_SSL` to be enabled.*

### Invoking GitHub REST API

Listing GitHub public events is not a big deal:

```cpp
auto r = session.Send(webcc::RequestBuilder{}.
                      Get("https://api.github.com/events")
                      ());
```

You can then parse `r->data()` to JSON object with your favorite JSON library. My choice for the examples is [jsoncpp](https://github.com/open-source-parsers/jsoncpp). But Webcc itself doesn't understand JSON nor require one. It's up to you to choose the most appropriate JSON library.

`RequestBuilder` provides a lot of functions for you to customize the request. Let's see more examples.

### Authorization

In order to list the followers of an authorized GitHub user, you need either **Basic Authorization**:

```cpp
session.Send(webcc::RequestBuilder{}.
             Get("https://api.github.com/user/followers").
             AuthBasic(<login>, <password>)
             ());
```

Or **Token Authorization**:

```cpp
session.Send(webcc::RequestBuilder{}.
             Get("https://api.github.com/user/followers").
             AuthToken(<token>)
             ());
```

### Keep-Alive

Though **Keep-Alive** (i.e., Persistent Connection) is a good feature and enabled by default, you can turn it off:

```cpp
auto r = session.Send(webcc::RequestBuilder{}.
                      Get("http://httpbin.org/get").
                      KeepAlive(false)  // No Keep-Alive
                      ());
```

The API for other HTTP requests is no different from GET.

### POST Request

POST request needs a body which is normally a JSON string for REST API. Let's post a small UTF-8 encoded JSON string:

```cpp
session.Send(webcc::RequestBuilder{}.
             Post("http://httpbin.org/post").
             Body("{'name'='Adam', 'age'=20}").Json().Utf8()
             ());
```

### Downloading Files

Webcc has the ability to stream large response data to a file. This is especially useful when downloading files.

```cpp
auto r = session.Send(webcc::RequestBuilder{}.
                      Get("http://httpbin.org/image/jpeg")
                      (), /*stream=*/true);

// Move the streamed file to your destination.
r->file_body()->Move("./wolf.jpeg");
```

### Uploading Files

Streaming is also available for uploading:

```cpp
auto r = session.Send(webcc::RequestBuilder{}.
                      Post("http://httpbin.org/post").
                      File(local/file/path)
                      ());
```

The file will not be loaded into the memory all at once, instead, it will be read and sent piece by piece.

Please note that `Content-Length` header will still be set to the true size of the file, this is different from the handling of chunked data (`Transfer-Encoding: chunked`).

### Client API and Threads

A `ClientSession` shouldn't be shared by multiple threads. Please create a new session for each thread.

Example:

```cpp
void ThreadedClient() {
  std::vector<std::thread> threads;

  for (int i = 0; i < 3; ++i) {
    threads.emplace_back([]() {
      webcc::ClientSession session;

      try {
        auto r = session.Send(webcc::RequestBuilder{}.
                              Get("http://httpbin.org/get")
                              ());
        std::cout << r->data() << std::endl;

      } catch (const webcc::Error&) {
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }
}
```

## Server API

### A Minimal Server

The following example is a minimal yet complete HTTP server.

Start it, open a browser with `localhost:8080`, you will see `Hello, World!` as response.

```cpp
#include "webcc/logger.h"
#include "webcc/response_builder.h"
#include "webcc/server.h"

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
    webcc::Server server{ boost::asio::ip::tcp::v4(), 8080 };

    server.Route("/", std::make_shared<HelloView>());

    server.Run();

  } catch (const std::exception&) {
    return 1;
  }

  return 0;
}
```

### URL Route

The `Route()` method routes different URLs to different `views`.

You can route different URLs to the same view:

```cpp
server.Route("/", std::make_shared<HelloView>());
server.Route("/hello", std::make_shared<HelloView>());
```

Or even the same view object:

```cpp
auto view = std::make_shared<HelloView>();
server.Route("/", view);
server.Route("/hello", view);
```

But normally a view only handles a specific URL (see the Book Server example). 

The URL could be regular expressions. The Book Server example uses a regex URL to match against book IDs.

Finally, it's always suggested to explicitly specify the HTTP methods allowed for a route:

```cpp
server.Route("/", std::make_shared<HelloView>(), { "GET" });
```

### Running A Server

The last thing about server is `Run()`:

```cpp
void Run(std::size_t workers = 1, std::size_t loops = 1);
```

Workers are threads which will be waken to process the HTTP requests once they arrive. Theoretically, the more `workers` you have, the more concurrency you gain. In practice, you have to take the number of CPU cores into account and allocate a reasonable number for it.

The `loops` means the number of threads running the IO Context of Asio. Normally, one thread is good enough, but it could be more than that.

### Response Builder

The server API provides a helper class `ResponseBuilder` for the views to chain the parameters and finally build a response object. This is exactly the same strategy as `RequestBuilder`.

### REST Book Server

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
    webcc::Server server{ boost::asio::ip::tcp::v4(), 8080 };

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

Please see [examples/book_server](examples/book_server) for more details.

## IPv6 Support

### IPv6 Server

Only need to change the protocol to `boost::asio::ip::tcp::v6()`:

```cpp
webcc::Server server{ boost::asio::ip::tcp::v6(), 8080 };
```

### IPv6 Client

Only need to specify an IPv6 address:

```cpp
auto r = session.Send(webcc::RequestBuilder{}.
                      Get("http://[::1]:8080/books").
                      ());
```
