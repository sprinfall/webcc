# webcc

基于 [Boost Asio](https://www.boost.org/doc/libs/release/libs/asio/) 开发的轻量级 C++ HTTP 程序库，同时支持客户端与服务端。 

[编译指南](https://github.com/sprinfall/webcc/wiki/Build-Instructions)，目前只有英文版。

代码仓库: [https://github.com/sprinfall/webcc](https://github.com/sprinfall/webcc)。请认准链接，其他人 fork 的仓库，都不是最新的。

**功能概述**

- 跨平台: Windows，Linux 及 MacOS
- 简单好用的客户端 API，借鉴了 Python 的 [requests](https://2.python-requests.org//en/master/) 程序库
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
    auto r = session.Get("http://httpbin.org/get");

    // 输出响应数据
    std::cout << r->data() << std::endl;

  } catch (const webcc::Error& error) {
    // 异常处理
    std::cerr << error << std::endl;
  }

  return 0;
}
```

因为 `Get()` 不过是 `Request()` 的一种快捷方式，直接调用 `Request()` 会更复杂一些：

```cpp
auto r = session.Request(webcc::RequestBuilder{}.Get("http://httpbin.org/get")());
```

这里多了个辅助类 `RequestBuilder`，用来串联起各种参数，最后再生成一个请求对象。注意不要漏了 `()` 操作符。

不管是 `Get` 还是 `Request()`，都接受 URL 查询参数：

```cpp
// 查询参数由 std::vector 一起指定，键值成对出现
session.Get("http://httpbin.org/get", { "key1", "value1", "key2", "value2" });

// 查询参数由 Query() 挨个指定
session.Request(webcc::RequestBuilder{}.
                Get("http://httpbin.org/get").
                Query("key1", "value1").
                Query("key2", "value2")
                ());
```

要添加额外的头部也很简单：

```cpp
session.Get("http://httpbin.org/get",
            {"key1", "value1", "key2", "value2"},
            {"Accept", "application/json"});  // 也是 std::vector
                
session.Request(webcc::RequestBuilder{}.
                Get("http://httpbin.org/get").
                Query("key1", "value1").
                Query("key2", "value2").
                Header("Accept", "application/json")
                ());
```

访问 HTTPS 和访问 HTTP 没有差别，对用户是透明的：

```cpp
session.Get("https://httpbin.org/get");
```

*注意：对 HTTPS/SSL 的支持，需要启用编译选项 `WEBCC_ENABLE_SSL`，也会依赖 OpenSSL。*

列出 GitHub 公开事件 (public events) 也不是什么难题：

```cpp
auto r = session.Get("https://api.github.com/events");
```

然后，你可以把 `r->data()` 解析成 JSON 对象，随便用个什么 JSON 程序库。

我在示例程序里用的是 [jsoncpp](https://github.com/open-source-parsers/jsoncpp)，但是 Webcc 本身并不理解 JSON，用什么 JSON 程序库，完全是你自己的选择。

快捷函数（`Get()`，`Post()`，等）用起来方便，但是参数有限，限制比较多。`RequestBuilder` 更灵活更强大，它提供了很多函数供你定制请求的样子。

为了列出一个授权的 (authorized) GitHub 用户的“粉丝” (followers)，要么使用  **Basic 认证**：

```cpp
session.Request(webcc::RequestBuilder{}.
                Get("https://api.github.com/user/followers").
                AuthBasic(login, password)  // 应该替换成具体的账号、密码
                ());
```

要么使用 **Token 认证**：

```cpp
session.Request(webcc::RequestBuilder{}.
                Get("https://api.github.com/user/followers").
                AuthToken(token)  // 应该替换成具体合法的 token
                ());
```

尽管**持久连接** (Keep-Alive) 这个功能不错，你也可以手动关掉它：

```cpp
auto r = session.Request(webcc::RequestBuilder{}.
                         Get("http://httpbin.org/get").
                         KeepAlive(false)  // 不要 Keep-Alive
                         ());
```

其他 HTTP 请求的 API 跟 GET 并无太多差别。

POST 请求需要一个“体” (body)，就 REST API 来说通常是一个 JSON 字符串。让我们 POST 一个 UTF-8 编码的 JSON 字符串：

```cpp
session.Request(webcc::RequestBuilder{}.
                Post("http://httpbin.org/post").
                Body("{'name'='Adam', 'age'=20}").
                Json().Utf8()
                ());
```

Webcc 可以把大型的响应数据串流到临时文件，串流在下载文件时特别有用。

```cpp
auto r = session.Request(webcc::RequestBuilder{}.
                         Get("http://httpbin.org/image/jpeg")(),
                         true);  // stream = true

// 把串流的文件移到目标位置
r->file_body()->Move("./wolf.jpeg");
```

不光下载，上传也可以串流：

```cpp
auto r = session.Request(webcc::RequestBuilder{}.
                         Post("http://httpbin.org/post").
                         File(path)  // 应该替换成具体的文件路径
                         ());
```

这个文件在 POST 时，不会一次加载到内存，读一块数据发一块数据，直到发送完。

注意，`Content-Length` 头部还是会设置为文件的真实大小，不同于 `Transfer-Encoding: chunked` 的分块数据形式。

更多示例和用法，请参考 [examples](https://github.com/sprinfall/webcc/tree/master/examples/) 目录。 

## 服务端 API

### Hello, World!

下面是个 `Hello, World!` 级别的服务程序。
程序运行后，打开浏览器，输入 `http://localhost:8080`，页面显示 `Hello, World!`。

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

简单解释一下。一个服务器 (server) 对应多个视图 (view)，不同的视图对应不同的资源，视图通过 URL 路由，且 URL 可以为正则表达式。

完整代码请见 [examples/hello_world_server](https://github.com/sprinfall/webcc/tree/master/examples/hello_world_server.cc)。 

下面看一个更复杂的例子。

### 在线书店

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
    webcc::Server server(8080);

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

完整实现请见 [examples/rest_book_server.cc](https://github.com/sprinfall/webcc/tree/master/examples/rest_book_server.cc)。
