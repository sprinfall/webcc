HttpBin (http://httpbin.org/) client example.

You request to different endpoints, and it returns information about what was in the request.

E.g., request:
```plain
GET /get HTTP/1.1
Host: httpbin.org:80
User-Agent: Webcc/0.1.0

```

Response:
```plain
HTTP/1.1 200 OK
Connection: keep-alive
Server: gunicorn/19.9.0
Content-Type: application/json
Content-Length: 191
Access-Control-Allow-Origin: *
Access-Control-Allow-Credentials: true
Via: 1.1 vegur

{
  "args": {},
  "headers": {
    "Connection": "close",
    "Host": "httpbin.org",
    "User-Agent": "Webcc/0.1.0"
  },
  "origin": "198.55.94.81",
  "url": "http://httpbin.org/get"
}
```

As you can see, the request information is returned in JSON format.
