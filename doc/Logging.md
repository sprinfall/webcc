# Logging

## Log Control

Webcc's logging module was designed for "best performance".

By defining macro `WEBCC_ENABLE_LOG` as `1` or `0`, you can enable or disable the logging globally at compile-time. And by setting `WEBCC_LOG_LEVEL` to `0` to `4`, you can control above which level the logs will be logged.

There are five log levels defined in webcc:

```cpp
// Log levels.
// VERB is similar to DEBUG commonly used by other projects.
// USER is for the users who want to log their own logs but don't want any
// VERB or INFO.
#define WEBCC_VERB 0
#define WEBCC_INFO 1
#define WEBCC_USER 2
#define WEBCC_WARN 3
#define WEBCC_ERRO 4
```

One important difference from other logging libraries is that the level control is at compile-time. For example, when you define `WEBCC_LOG_LEVEL` to `2` (`USER`), the logs of `VERB` and `INFO` levels will be totally eliminated during preprocessing of the compiler.

### Configure With CMake

If you are using _CMake_, you can define the macros in your _CMakeLists.txt_. Take webcc's own CMake files as example, they are defined in the _CMakeLists.txt_ of the project root directory. And in order to configure dynamically, they are determined by corresponding CMake variables:

```cmake
set(WEBCC_ENABLE_LOG 1 CACHE STRING "Enable logging? (0:OFF, 1:ON)")
set(WEBCC_LOG_LEVEL 2 CACHE STRING "Log level (0:VERB, 1:INFO, 2:USER, 3:WARN or 4:ERRO)")
```

These two CMake variables determine the related macros defined in `config.h`. The file `config.h` will be generated automatically during CMake configure step from `webcc/config.in`.

```cmake
# webcc/CMakeLists.txt
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )
```

### Configure With Visual Studio

If you manage projects with Visual Studio directly, you just create `config.h` by copying `config.h.example` then change the values accordingly.

### Set Log Level Properly

During development, set log level to `VERB` to log as many as possible so that you can find more issues. But before release, please switch log level to at least `USER` for better performance. Verbose logs consume a lot of memory, IO and CPU. Never enable `VERB` in a release!

## Log Format

For simplicity, the format of the logs is not configurable. It consists of the following fields:
```plain
Timestamp, Level, Thread ID, File Name, Line Number, Message
```
Each field is well aligned and formatted to achieve a beautiful display. See the following examples.

### Example: Server Side

The following logs are from [book_server](../examples/book_server) example.

```csv
2019-07-11 15:25:35.620, INFO,    main,            server.cc,   60, Server is going to run...
2019-07-11 15:25:35.635, INFO,    4160,   request_handler.cc,   89, Worker is running.
2019-07-11 15:25:35.635, INFO,    2116,   request_handler.cc,   89, Worker is running.
2019-07-11 15:25:53.148, INFO,    main,            server.cc,   95, Accepted a connection.
2019-07-11 15:25:53.148, VERB,    main,   connection_pool.cc,    8, Starting connection...
2019-07-11 15:25:53.149, INFO,    main,            parser.cc,   61, HTTP headers will continue in next read.
2019-07-11 15:25:53.150, INFO,    main,            parser.cc,   65, HTTP headers just ended.
2019-07-11 15:25:53.150, VERB,    main,        connection.cc,   94, HTTP request:
    > GET /books HTTP/1.1
    > User-Agent: Webcc/0.1.0
    > Accept-Encoding: gzip, deflate
    > Accept: */*
    > Connection: Keep-Alive
    > Host: localhost:8080
    >

2019-07-11 15:25:53.151, INFO,    2116,   request_handler.cc,  114, Request URL path: /books
2019-07-11 15:25:53.153, VERB,    2116,        connection.cc,  102, HTTP response:
    > HTTP/1.1 200 OK
    > Content-Type: application/json; charset=utf-8
    > Content-Length: 2
    > Connection: Keep-Alive
    > Server: Webcc/0.1.0
    >
    > []

2019-07-11 15:25:53.155, INFO,    main,        connection.cc,  146, Response has been sent back.
2019-07-11 15:25:53.155, INFO,    main,        connection.cc,  149, The client asked for a keep-alive connection.
2019-07-11 15:25:53.156, INFO,    main,        connection.cc,  150, Continue to read the next request...
2019-07-11 15:25:53.212, INFO,    main,            parser.cc,  156, Content length: 52.
2019-07-11 15:25:53.214, INFO,    main,            parser.cc,   61, HTTP headers will continue in next read.
2019-07-11 15:25:53.214, INFO,    main,            parser.cc,   65, HTTP headers just ended.
2019-07-11 15:25:53.215, VERB,    main,        connection.cc,   94, HTTP request:
    > POST /books HTTP/1.1
    > Content-Type: application/json
    > Content-Length: 52
    > User-Agent: Webcc/0.1.0
    > Accept-Encoding: gzip, deflate
    > Accept: */*
    > Connection: Keep-Alive
    > Host: localhost:8080
    >
    > {
    >   "price" : 12.300000000000001,
    >   "title" : "1984"
    > }

2019-07-11 15:25:53.222, INFO,    2116,   request_handler.cc,  114, Request URL path: /books
2019-07-11 15:25:53.225, VERB,    2116,        connection.cc,  102, HTTP response:
    > HTTP/1.1 201 Created
    > Content-Type: application/json; charset=utf-8
    > Content-Length: 15
    > Connection: Keep-Alive
    > Server: Webcc/0.1.0
    >
    > {
    >   "id" : "1"
    > }

2019-07-11 15:25:53.241, INFO,    main,        connection.cc,  146, Response has been sent back.
2019-07-11 15:25:53.245, INFO,    main,        connection.cc,  149, The client asked for a keep-alive connection.
2019-07-11 15:25:53.253, INFO,    main,        connection.cc,  150, Continue to read the next request...
2019-07-11 15:25:53.358, INFO,    main,            parser.cc,   61, HTTP headers will continue in next read.
2019-07-11 15:25:53.360, INFO,    main,            parser.cc,   65, HTTP headers just ended.
2019-07-11 15:25:53.362, VERB,    main,        connection.cc,   94, HTTP request:
    > GET /books HTTP/1.1
    > User-Agent: Webcc/0.1.0
    > Accept-Encoding: gzip, deflate
    > Accept: */*
    > Connection: Keep-Alive
    > Host: localhost:8080
    >

2019-07-11 15:25:53.372, INFO,    2116,   request_handler.cc,  114, Request URL path: /books
2019-07-11 15:25:53.379, VERB,    2116,        connection.cc,  102, HTTP response:
    > HTTP/1.1 200 OK
    > Content-Type: application/json; charset=utf-8
    > Content-Length: 74
    > Connection: Keep-Alive
    > Server: Webcc/0.1.0
    >
    > [
    >   {
    >           "id" : "1",
    >           "price" : 12.300000000000001,
    >           "title" : "1984"
    >   }
    > ]

2019-07-11 15:25:53.399, INFO,    main,        connection.cc,  146, Response has been sent back.
2019-07-11 15:25:53.400, INFO,    main,        connection.cc,  149, The client asked for a keep-alive connection.
2019-07-11 15:25:53.407, INFO,    main,        connection.cc,  150, Continue to read the next request...
...
```

As you can see, the HTTP requests and responses are logged at level `VERB`, and they are indented by 4 spaces and a `"> "` prefix to improve the readability. The thread ID will be replaced by "main" if it's logged from main thread.

### Example: Client Side

The following logs are from [book_client](../examples/book_client) example.

```csv
2019-07-11 15:25:53.052, VERB,    main,            client.cc,   80, Resize buffer: 0 -> 1024.
2019-07-11 15:25:53.146, VERB,    main,            client.cc,  117, Connect to server...
2019-07-11 15:25:53.147, VERB,    main,            client.cc,  128, Socket connected.
2019-07-11 15:25:53.148, VERB,    main,            client.cc,  132, HTTP request:
    > GET /books HTTP/1.1
    > User-Agent: Webcc/0.1.0
    > Accept-Encoding: gzip, deflate
    > Accept: */*
    > Connection: Keep-Alive
    > Host: localhost:8080
    >

2019-07-11 15:25:53.149, INFO,    main,            client.cc,  161, Request sent.
2019-07-11 15:25:53.149, VERB,    main,            client.cc,  165, Read response (timeout: 30s)...
2019-07-11 15:25:53.150, VERB,    main,            client.cc,  238, Wait timer asynchronously.
2019-07-11 15:25:53.154, VERB,    main,            client.cc,  184, Socket async read handler.
2019-07-11 15:25:53.155, INFO,    main,            client.cc,  274, Cancel timer...
2019-07-11 15:25:53.156, INFO,    main,            client.cc,  197, Read data, length: 115.
2019-07-11 15:25:53.157, INFO,    main,            common.cc,  127, Content-type charset: utf-8.
2019-07-11 15:25:53.158, INFO,    main,            parser.cc,  156, Content length: 2.
2019-07-11 15:25:53.159, INFO,    main,            parser.cc,   61, HTTP headers will continue in next read.
2019-07-11 15:25:53.161, VERB,    main,            client.cc,  243, On timer.
2019-07-11 15:25:53.162, VERB,    main,            client.cc,  247, Timer canceled.
2019-07-11 15:25:53.163, VERB,    main,            client.cc,  184, Socket async read handler.
2019-07-11 15:25:53.164, INFO,    main,            client.cc,  197, Read data, length: 17.
2019-07-11 15:25:53.178, INFO,    main,            parser.cc,   65, HTTP headers just ended.
2019-07-11 15:25:53.179, INFO,    main,            client.cc,  213, Keep the socket connection alive.
2019-07-11 15:25:53.180, INFO,    main,            client.cc,  218, Finished to read and parse HTTP response.
2019-07-11 15:25:53.181, VERB,    main,            client.cc,  173, HTTP response:
    > HTTP/1.1 200 OK
    > Content-Type: application/json; charset=utf-8
    > Content-Length: 2
    > Connection: Keep-Alive
    > Server: Webcc/0.1.0
    >
    > []

2019-07-11 15:25:53.183, INFO,    main,       client_pool.cc,   32, Added connection to pool (http, localhost, 8080).
2019-07-11 15:25:53.206, VERB,    main,    client_session.cc,  191, Reuse an existing connection.
2019-07-11 15:25:53.207, VERB,    main,            client.cc,  132, HTTP request:
    > POST /books HTTP/1.1
    > Content-Type: application/json
    > Content-Length: 52
    > User-Agent: Webcc/0.1.0
    > Accept-Encoding: gzip, deflate
    > Accept: */*
    > Connection: Keep-Alive
    > Host: localhost:8080
    >
    > {
    >   "price" : 12.300000000000001,
    >   "title" : "1984"
    > }

2019-07-11 15:25:53.212, INFO,    main,            client.cc,  161, Request sent.
2019-07-11 15:25:53.219, VERB,    main,            client.cc,  165, Read response (timeout: 30s)...
2019-07-11 15:25:53.220, VERB,    main,            client.cc,  238, Wait timer asynchronously.
2019-07-11 15:25:53.240, VERB,    main,            client.cc,  184, Socket async read handler.
2019-07-11 15:25:53.241, INFO,    main,            client.cc,  274, Cancel timer...
2019-07-11 15:25:53.246, INFO,    main,            client.cc,  197, Read data, length: 121.
2019-07-11 15:25:53.253, INFO,    main,            common.cc,  127, Content-type charset: utf-8.
2019-07-11 15:25:53.262, INFO,    main,            parser.cc,  156, Content length: 15.
2019-07-11 15:25:53.271, INFO,    main,            parser.cc,   61, HTTP headers will continue in next read.
2019-07-11 15:25:53.278, VERB,    main,            client.cc,  243, On timer.
2019-07-11 15:25:53.280, VERB,    main,            client.cc,  247, Timer canceled.
2019-07-11 15:25:53.284, VERB,    main,            client.cc,  184, Socket async read handler.
2019-07-11 15:25:53.288, INFO,    main,            client.cc,  197, Read data, length: 30.
2019-07-11 15:25:53.296, INFO,    main,            parser.cc,   65, HTTP headers just ended.
2019-07-11 15:25:53.298, INFO,    main,            client.cc,  213, Keep the socket connection alive.
2019-07-11 15:25:53.312, INFO,    main,            client.cc,  218, Finished to read and parse HTTP response.
2019-07-11 15:25:53.314, VERB,    main,            client.cc,  173, HTTP response:
    > HTTP/1.1 201 Created
    > Content-Type: application/json; charset=utf-8
    > Content-Length: 15
    > Connection: Keep-Alive
    > Server: Webcc/0.1.0
    >
    > {
    >   "id" : "1"
    > }

2019-07-11 15:25:53.344, VERB,    main,    client_session.cc,  191, Reuse an existing connection.
2019-07-11 15:25:53.344, VERB,    main,            client.cc,  132, HTTP request:
    > GET /books HTTP/1.1
    > User-Agent: Webcc/0.1.0
    > Accept-Encoding: gzip, deflate
    > Accept: */*
    > Connection: Keep-Alive
    > Host: localhost:8080
    >

2019-07-11 15:25:53.357, INFO,    main,            client.cc,  161, Request sent.
2019-07-11 15:25:53.362, VERB,    main,            client.cc,  165, Read response (timeout: 30s)...
2019-07-11 15:25:53.369, VERB,    main,            client.cc,  238, Wait timer asynchronously.
2019-07-11 15:25:53.398, VERB,    main,            client.cc,  184, Socket async read handler.
2019-07-11 15:25:53.400, INFO,    main,            client.cc,  274, Cancel timer...
2019-07-11 15:25:53.407, INFO,    main,            client.cc,  197, Read data, length: 116.
2019-07-11 15:25:53.412, INFO,    main,            common.cc,  127, Content-type charset: utf-8.
2019-07-11 15:25:53.415, INFO,    main,            parser.cc,  156, Content length: 74.
2019-07-11 15:25:53.418, INFO,    main,            parser.cc,   61, HTTP headers will continue in next read.
2019-07-11 15:25:53.421, VERB,    main,            client.cc,  243, On timer.
2019-07-11 15:25:53.423, VERB,    main,            client.cc,  247, Timer canceled.
2019-07-11 15:25:53.431, VERB,    main,            client.cc,  184, Socket async read handler.
2019-07-11 15:25:53.433, INFO,    main,            client.cc,  197, Read data, length: 89.
2019-07-11 15:25:53.440, INFO,    main,            parser.cc,   65, HTTP headers just ended.
2019-07-11 15:25:53.441, INFO,    main,            client.cc,  213, Keep the socket connection alive.
2019-07-11 15:25:53.445, INFO,    main,            client.cc,  218, Finished to read and parse HTTP response.
2019-07-11 15:25:53.448, VERB,    main,            client.cc,  173, HTTP response:
    > HTTP/1.1 200 OK
    > Content-Type: application/json; charset=utf-8
    > Content-Length: 74
    > Connection: Keep-Alive
    > Server: Webcc/0.1.0
    >
    > [
    >   {
    >           "id" : "1",
    >           "price" : 12.300000000000001,
    >           "title" : "1984"
    >   }
    > ]

2019-07-11 15:25:53.473, VERB,    main,    client_session.cc,  191, Reuse an existing connection.
2019-07-11 15:25:53.474, VERB,    main,            client.cc,  132, HTTP request:
    > GET /books/1 HTTP/1.1
    > User-Agent: Webcc/0.1.0
    > Accept-Encoding: gzip, deflate
    > Accept: */*
    > Connection: Keep-Alive
    > Host: localhost:8080
    >

2019-07-11 15:25:53.481, INFO,    main,            client.cc,  161, Request sent.
2019-07-11 15:25:53.486, VERB,    main,            client.cc,  165, Read response (timeout: 30s)...
2019-07-11 15:25:53.488, VERB,    main,            client.cc,  238, Wait timer asynchronously.
2019-07-11 15:25:53.498, VERB,    main,            client.cc,  184, Socket async read handler.
2019-07-11 15:25:53.508, INFO,    main,            client.cc,  274, Cancel timer...
2019-07-11 15:25:53.510, INFO,    main,            client.cc,  197, Read data, length: 116.
2019-07-11 15:25:53.512, INFO,    main,            common.cc,  127, Content-type charset: utf-8.
2019-07-11 15:25:53.515, INFO,    main,            parser.cc,  156, Content length: 65.
2019-07-11 15:25:53.517, INFO,    main,            parser.cc,   61, HTTP headers will continue in next read.
2019-07-11 15:25:53.522, VERB,    main,            client.cc,  243, On timer.
2019-07-11 15:25:53.524, VERB,    main,            client.cc,  247, Timer canceled.
2019-07-11 15:25:53.526, VERB,    main,            client.cc,  184, Socket async read handler.
2019-07-11 15:25:53.527, INFO,    main,            client.cc,  197, Read data, length: 80.
2019-07-11 15:25:53.529, INFO,    main,            parser.cc,   65, HTTP headers just ended.
2019-07-11 15:25:53.540, INFO,    main,            client.cc,  213, Keep the socket connection alive.
2019-07-11 15:25:53.550, INFO,    main,            client.cc,  218, Finished to read and parse HTTP response.
2019-07-11 15:25:53.553, VERB,    main,            client.cc,  173, HTTP response:
    > HTTP/1.1 200 OK
    > Content-Type: application/json; charset=utf-8
    > Content-Length: 65
    > Connection: Keep-Alive
    > Server: Webcc/0.1.0
    >
    > {
    >   "id" : "1",
    >   "price" : 12.300000000000001,
    >   "title" : "1984"
    > }
...
```

### Colorful Console Output

If the terminal supports colors, some levels of the logs will be highlighted.

![Logger colors](screenshots/logger_colors.png)

## Initialize Logging

In your program, call macro `WEBCC_LOG_INIT()` to initialize the logging. E.g., the following example initializes logging to write to both console and file. The file path is the current directory (indicated by passing empty string to the first parameter), and the file will be overwritten instead of appended.

```cpp
#include "webcc/logger.h"

WEBCC_LOG_INIT("", webcc::LOG_CONSOLE_FILE_OVERWRITE);
```

Constant `LOG_CONSOLE_FILE_OVERWRITE` is actually a combination of several mode flags:
```cpp
const int LOG_CONSOLE_FILE_OVERWRITE = LOG_CONSOLE | LOG_FILE | LOG_OVERWRITE;
```

And the flags are defined as `enum` values:
```cpp
enum LogMode {
  LOG_FILE        = 1,  // Log to file.
  LOG_CONSOLE     = 2,  // Log to console.
  LOG_FLUSH       = 4,  // Flush on each log.
  LOG_OVERWRITE   = 8,  // Overwrite any existing log file.
};
```
