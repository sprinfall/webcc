# Webcc - C++ HTTP 程序库

**注意：[legacy](https://github.com/sprinfall/webcc/tree/legacy) 分支只使用了有限的 C++11 特性，能支持较老的编译器，比如 VS2013 和 GCC 4.8。**

基于 [Boost Asio](https://www.boost.org/doc/libs/release/libs/asio/) 开发的轻量级 C++ HTTP 程序库，同时支持客户端与服务端。

不管你是要访问 HTTP 服务（比如调用 REST API、下载一个文件），还是要在你的程序里嵌入一个 HTTP 服务（比如 REST Server），Webcc 都是个不错的选择。

Boost Beast 没有一个开箱即用的 HTTP Server，微软 cpprest 的 API 设计复杂，且 server 部分也几乎不可用。Webcc 能满足大多数需求，又兼顾了性能和代码质量。这一点你看一下我们的代码心里就有数了。

=> [编译指南](doc/Build-Instructions_zh_CN.md)

代码仓库: [https://github.com/sprinfall/webcc](https://github.com/sprinfall/webcc)。 请认准链接，其他人 fork 的仓库，都不是最新的。

**内容**
* [功能概述](#功能概述)
* [客户端 API](#客户端-api)
    * [一个完整的例子](#一个完整的例子)
    * [Request Builder](#request-builder)
    * [HTTPS](#https)
    * [调用 GitHub REST API](#调用-github-rest-api)
    * [Authorization](#authorization)
    * [Keep-Alive](#keep-alive)
    * [POST 请求](#post-请求)
    * [下载文件](#下载文件)
    * [上传文件](#上传文件)
    * [多线程调用](#多线程调用)
* [服务端 API](#服务端-api)
    * [一个最小的服务器](#一个最小的服务器)
    * [URL 路由](#url-路由)
    * [运行服务器](#运行服务器)
    * [Response Builder](#response-builder)
    * [REST Book Server](#rest-book-server)
* [IPv6 支持](#ipv6-支持)
    * [IPv6 服务端](#ipv6-服务端)
    * [IPv6 客户端](#ipv6-客户端)

## 功能概述

- 跨平台: Windows，Linux 及 MacOS
- 简单好用的客户端 API，借鉴了 Python 的 [requests](https://2.python-requests.org//en/master/) 程序库
- 支持 IPv6
- 支持 SSL/HTTPS，依赖 OpenSSL（可选）
- 支持 GZip 压缩，依赖 Zlib（可选）
- 持久连接 (Keep-Alive)
- 数据串流 (Streaming)
    - 客户端：可以上传、下载大型文件
    - 服务端：可以伺服、接收大型文件
- 支持 Basic & Token 认证/授权
- 超时控制（目前仅客户端）
- 代码遵守 [Google C++ Style](https://google.github.io/styleguide/cppguide.html)
- 自动化测试和单元测试保证质量
- 无内存泄漏（[VLD](https://kinddragon.github.io/vld/) 检测）

## 客户端 API

### 一个完整的例子

先来看一个完整的例子：

```cpp
#include <iostream>

#include "webcc/client_session.h"
#include "webcc/logger.h"

int main() {
  // 首先配置日志输出（到控制台/命令行）
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);
  
  // 创建会话
  webcc::ClientSession session;

  try {
    // 发起一个 HTTP GET 请求
    auto r = session.Send(webcc::RequestBuilder{}.
                          Get("http://httpbin.org/get")
                          ());

    // 输出响应数据
    std::cout << r->data() << std::endl;

  } catch (const webcc::Error& error) {
    // 异常处理
    std::cerr << error << std::endl;
  }

  return 0;
}
```

### Request Builder

如你所见，这里通过一个辅助类 `RequestBuilder`，串联起各种参数，最后再生成一个请求对象。注意不要漏了最后的 `()` 操作符。

通过 `Query()` 可以方便地指定 URL 查询参数：

```cpp
session.Send(webcc::RequestBuilder{}.
             Get("http://httpbin.org/get").
             Query("key1", "value1").Query("key2", "value2")
             ());
```

要添加额外的头部也很简单：

```cpp
session.Send(webcc::RequestBuilder{}.
             Get("http://httpbin.org/get").
             Header("Accept", "application/json")
             ());
```

### HTTPS

访问 HTTPS 和访问 HTTP 没有差别，对用户是透明的：

```cpp
session.Send(webcc::RequestBuilder{}.Get("https://httpbin.org/get")());
```

*注意：对 HTTPS/SSL 的支持，需要启用编译选项 `WEBCC_ENABLE_SSL`，也会依赖 OpenSSL。*

### 调用 GitHub REST API

列出 GitHub 公开事件 (public events) 也不是什么难题：

```cpp
auto r = session.Send(webcc::RequestBuilder{}.
                      Get("https://api.github.com/events")
                      ());
```

然后，你可以把 `r->data()` 解析成 JSON 对象，随便用个什么 JSON 程序库。

我在示例程序里用的是 [jsoncpp](https://github.com/open-source-parsers/jsoncpp)，但是 Webcc 本身并不理解 JSON，用什么 JSON 程序库，完全是你自己的选择。

`RequestBuilder` 本质上是为了解决 C++ 没有“键值参数”的问题，它提供了很多函数供你定制请求的样子。

### Authorization

为了列出一个授权的 (authorized) GitHub 用户的“粉丝” (followers)，要么使用 **Basic 认证**：

```cpp
session.Send(webcc::RequestBuilder{}.
             Get("https://api.github.com/user/followers").
             AuthBasic(login, password)  // 应该替换成具体的账号、密码
             ());
```

要么使用 **Token 认证**：

```cpp
session.Send(webcc::RequestBuilder{}.
             Get("https://api.github.com/user/followers").
             AuthToken(token)  // 应该替换成具体合法的 token
             ());
```

### Keep-Alive

尽管 **持久连接** (Keep-Alive) 这个功能不错，你也可以手动关掉它：

```cpp
auto r = session.Send(webcc::RequestBuilder{}.
                      Get("http://httpbin.org/get").
                      KeepAlive(false)  // 不要 Keep-Alive
                      ());
```

其他 HTTP 请求的 API 跟 GET 并无太多差别。

### POST 请求

POST 请求需要一个“体” (body)，就 REST API 来说通常是一个 JSON 字符串。让我们 POST 一个 UTF-8 编码的 JSON 字符串：

```cpp
session.Send(webcc::RequestBuilder{}.
             Post("http://httpbin.org/post").
             Body("{'name'='Adam', 'age'=20}").Json().Utf8()
             ());
```

除了 JSON 字符串，POST 请求的体可以为任何内容。它可以是一个文件的二进制内容，见：[上传文件](#上传文件)。它也可以是一个 URL 编码的字符串：

```cpp
session.SetContentType("application/x-www-form-urlencoded", "utf8");

// 使用 UrlQuery 来组装 URL 编码字符串。
// 不要使用 RequestBuilder::Query()，那是专门给 GET 请求用的。
webcc::UrlQuery query;
query.Add("key1", "value1");
query.Add("key2", "value2");
// ...

auto r = session.Send(webcc::RequestBuilder{}.
                      Post("http://httpbin.org/post").
                      Body(query.ToString())
                      ());
```

更多细节请参见：[examples/form_urlencoded_client.cc](examples/form_urlencoded_client.cc)。

### 下载文件

Webcc 可以把大型的响应数据串流到临时文件，串流在下载文件时特别有用。

```cpp
auto r = session.Send(webcc::RequestBuilder{}.
                      Get("http://httpbin.org/image/jpeg")
                      (), true);  // stream = true

// 把串流的文件移到目标位置
r->file_body()->Move("./wolf.jpeg");
```

### 上传文件

不光下载，上传也可以串流：

```cpp
auto r = session.Send(webcc::RequestBuilder{}.
                      Post("http://httpbin.org/post").
                      File(path)  // 应该替换成具体的文件路径
                      ());
```

这个文件在 POST 时，不会一次加载到内存，而是读一块数据发一块数据，直到发送完。

注意，`Content-Length` 头部还是会设置为文件的真实大小，不同于 `Transfer-Encoding: chunked` 的分块数据形式。

### 多线程调用

一个 `ClientSession` 对象同时只能给一个线程使用，不可以在多个线程间共享。

多线程调用示例:

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

## 服务端 API

### 一个最小的服务器

下面是个 `Hello, World!` 级别的服务程序。
程序运行后，打开浏览器，输入 `http://localhost:8080`，页面显示 `Hello, World!`。

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

### URL 路由

通过 `Route()` 方法，不同的 URLs 被路由到不同的**视图**。

你也可以路由不同的 URLs 到同一个视图：

```cpp
server.Route("/", std::make_shared<HelloView>());
server.Route("/hello", std::make_shared<HelloView>());
```

甚至路由到同一个视图对象：

```cpp
auto view = std::make_shared<HelloView>();
server.Route("/", view);
server.Route("/hello", view);
```

但是一般情况下，一个视图只处理一种特定的 URL（请参考 Book Server 示例）。

URL 可以是**正则表达式**。Book Server 示例用了一个正则表达式的 URL 来匹配书本的 ID。

最后，强烈建议你总是显式地指定一个路由所允许的 HTTP 方法：

```cpp
server.Route("/", std::make_shared<HelloView>(), { "GET" });
```

### 运行服务器

关于服务器的最后一件事就是 `Run()`：

```cpp
void Run(std::size_t workers = 1, std::size_t loops = 1);
```

工作者 (`workers`) 代表那些会被唤醒去处理 HTTP 请求的线程。理论上，工作者越多，并发能力越强。实际操作时，你得考虑你有多少个可用的 CPU 内核，然后为它分配一个合理的数目。

第二个参数 (`loops`) 代表运行 Asio 上下文 (IO Context) 的线程个数。一般来说，一个线程就足够了，但是也可以为多个。

### Response Builder

服务端 API 提供了一个辅助类 `ResponseBuilder`，方便视图串联起各种参数，最后再生成一个响应对象。和 `RequestBuilder` 完全是同一种策略。

### REST Book Server

假定你想创建一个关于书的服务，提供下面这些 REST API：

- 查询书
- 添加一本新书
- 获取一本书的详情
- 更新一本书的信息
- 删除一本书

这是一组典型的 CRUD 操作。

前两个操作通过 `BookListView` 实现：

> ListView，DetailView 的命名方式，参考了 Django REST Framework。ListView 针对一列资源，DetailView 针对单个资源。

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
  // 查询书
  webcc::ResponsePtr Get(webcc::RequestPtr request);

  // 添加一本新书
  webcc::ResponsePtr Post(webcc::RequestPtr request);
};
```

其他操作通过 `BookDetailView` 实现：

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
  // 获取一本书的详情
  webcc::ResponsePtr Get(webcc::RequestPtr request);

  // 更新一本书的信息
  webcc::ResponsePtr Put(webcc::RequestPtr request);

  // 删除一本书
  webcc::ResponsePtr Delete(webcc::RequestPtr request);
};
```

我们挑一个函数出来看一下吧：

```cpp
webcc::ResponsePtr BookDetailView::Get(webcc::RequestPtr request) {
  if (request->args().size() != 1) {
    // NotFound (404) 意味着 URL 所指定的资源没有找到。
    // 这里用 BadRequest (400) 应该也是合理的。
    // 不过，后面可以看到，这个视图匹配了 "/books/(\\d+)" 这个 URL，参数肯定不会有问题的。
    // 所以这里的错误处理，只是出于防范，和编程的严谨。
    // Webcc 没有对 URL 参数做强类型的处理，那么代码写起来太复杂了。
    return webcc::ResponseBuilder{}.NotFound()();
  }

  const std::string& book_id = request->args()[0];

  // 通过 ID 找到这本书，比如，从数据库里。
  // ...

  if (<没找到>) {
    return webcc::ResponseBuilder{}.NotFound()();
  }

  // 把这本书转换成 JSON 字符串，并设为响应数据。
  return webcc::ResponseBuilder{}.OK().Data(<JsonStringOfTheBook>).
      Json().Utf8()();
}
```

最后一步，把 URLs 路由到特定的视图，然后开始运行：

```cpp
int main(int argc, char* argv[]) {
  // ...

  try {
    webcc::Server server{ boost::asio::ip::tcp::v4(), 8080 };

    server.Route("/books",
                 std::make_shared<BookListView>(),
                 { "GET", "POST" });

    // ID 通过正则表达式匹配出来
    server.Route(webcc::R("/books/(\\d+)"),
                 std::make_shared<BookDetailView>(),
                 { "GET", "PUT", "DELETE" });

    // 开始运行（注意：阻塞调用）
    server.Run();

  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
```

完整实现请见 [examples/book_server](examples/book_server)。

## IPv6 支持

### IPv6 服务端

只需把 Server 的 `protocol` 参数改成 `boost::asio::ip::tcp::v6()`：

```cpp
webcc::Server server{ boost::asio::ip::tcp::v6(), 8080 };
```

### IPv6 客户端

只需指定一个 IPv6 地址：

```cpp
auto r = session.Send(webcc::RequestBuilder{}.
                      Get("http://[::1]:8080/books").
                      ());
```
