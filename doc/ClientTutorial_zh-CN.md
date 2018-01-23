# cSoap 客户端使用指南

## 背景

首先，[gSoap](http://www.cs.fsu.edu/~engelen/soap.html) 肯定是个不错的选择，但是如果你的程序要调用多个 Web Services（即有多个 WSDL），gSoap 会比较麻烦。还有一个问题就是，gSoap 从 WSDL 自动生成的代码实在是太难用了。当然，这些都不是什么问题，真在的问题是许可证（License），gSoap 用在商业产品中是要收费的。

公司比较穷，舍不得花钱买 gSoap，但是 C++ 调 Web Service 还真没什么好办法。尝试了五六个半死不活的库后，最终锁定了 [WWSAPI](https://msdn.microsoft.com/en-us/library/windows/desktop/dd430435%28v=vs.85%29.aspx)（Windows Web Services API）。

WWSAPI 的官方文档经常让人摸不着头脑，没有完整的示例，给出一段代码，常常需要几经调整才能使用。WWSAPI 自动生成的代码，是纯 C 的接口，在难用程度上，较 gSoap 有过之而无不及。在消息参数上，它强制使用双字节 Unicode，我们的输入输出都是 UTF8 的 `std::string`，于是莫名地多出很多编码转换。WWSAPI 需要你手动分配堆（heap），需要你指定消息的缓冲大小，而最严重的问题是，它不够稳定，特别是在子线程里调用时，莫名其妙连接就会断掉。

于是，我就动手自己写了个 [cSoap](https://github.com/sprinfall/csoap)。

## 原理

cSoap 没有提供从 WSDL 自动生成代码的功能，一来是因为这一过程太复杂了，二来是自动生成的代码一般都不好用。所以 cSoap 最好搭配 [SoapUI](https://www.soapui.org) 一起使用。SoapUI 可以帮助我们为每一个 Web Service 操作（operation）生成请求的样例，基于请求样例，就很容易发起调用了，也避免了直接阅读 WSDL。

下面以 ParaSoft 提供的 [Calculator](http://ws1.parasoft.com/glue/calculator.wsdl) 为例，首先下载 WSDL，然后在 SoapUI 里创建一个 SOAP 项目，记得勾上 "Create sample requests for all operations?" 这个选项，然后就能看到下面这样的请求样例了：
```xml
<soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/" xmlns:cal="http://www.parasoft.com/wsdl/calculator/">
<soapenv:Header/>
<soapenv:Body>
   <cal:add>
      <cal:x>1</cal:x>
      <cal:y>2</cal:y>
   </cal:add>
</soapenv:Body>
</soapenv:Envelope>
```
这个操作是 "add"，有两个参数：x 和 y。此外值得注意的还有 XML namespace，比如 `xmlns:cal="http://www.parasoft.com/wsdl/calculator/"`。

要调用这个 "add" 操作，只要发一个 HTTP 请求，并把上面这个 SOAP Envelope 作为请求的 Content。在 SoapUI 里把 Request 切换到 “Raw" 模式，就可以看到下面这样完整的 HTTP 请求：
```
POST http://ws1.parasoft.com/glue/calculator HTTP/1.1
Accept-Encoding: gzip,deflate
Content-Type: text/xml;charset=UTF-8
SOAPAction: "add"
Content-Length: 300
Host: ws1.parasoft.com
Connection: Keep-Alive
User-Agent: Apache-HttpClient/4.1.1 (java 1.5)

<soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/" xmlns:cal="http://www.parasoft.com/wsdl/calculator/">
   <soapenv:Header/>
   <soapenv:Body>
      <cal:add>
         <cal:x>1</cal:x>
         <cal:y>1</cal:y>
      </cal:add>
   </soapenv:Body>
</soapenv:Envelope>
```
所以 cSoap 所做的，只不过是跟 `ws1.parasoft.com` 建立 TCP Socket 连接，然后发送上面这段内容而已。

## 用法

首先，创建一个类 `CalculatorClient`，继承自 `csoap::SoapClient`：

```cpp
#include <string>
#include "csoap/soap_client.h"

class CalculatorClient : public csoap::SoapClient {
public:
  CalculatorClient() {
    Init();
  }
```

在 `Init()` 函数里，初始化 URL、host、port 等等：
```cpp
private:
  void Init() {
    url_ = "/glue/calculator";
    host_ = "ws1.parasoft.com";
    port_ = "";  // Default to "80".
    service_ns_ = { "cal", "http://www.parasoft.com/wsdl/calculator/" };
    result_name_ = "Result";
  }
```

由于四个计算器操作（*add*, *subtract*, *multiply* 和 *divide*）都一致的具有两个参数，我们可以稍微封装一下，弄一个辅助函数叫 `Calc`：
```cpp
bool Calc(const std::string& operation,
          const std::string& x_name,
          const std::string& y_name,
          double x,
          double y,
          double* result) {
  // Prepare parameters.
  std::vector<csoap::Parameter> parameters{
    { x_name, x },
    { y_name, y }
  };

  // Make the call.
  std::string result_str;
  csoap::Error error = Call(operation, std::move(parameters), &result_str);
  
  // Error handling if any.
  if (error != csoap::kNoError) {
    std::cerr << "Error: " << error;
    std::cerr << ", " << csoap::GetErrorMessage(error) << std::endl;
    return false;
  }

  // Convert the result from string to double.
  try {
    *result = boost::lexical_cast<double>(result_str);
  } catch (boost::bad_lexical_cast&) {
    return false;
  }

  return true;
}
```

值得注意的是，作为局部变量的参数（parameters），利用了 C++11 的 Move 语义，避免了额外的拷贝开销。
当参数为很长的字符串时（比如 XML string），这一点特别有用。

最后，四个操作就是简单的转调 `Calc` 而已：
```cpp
bool Add(double x, double y, double* result) {
  return Calc("add", "x", "y", x, y, result);
}

bool Subtract(double x, double y, double* result) {
  return Calc("subtract", "x", "y", x, y, result);
}

bool Multiply(double x, double y, double* result) {
  return Calc("multiply", "x", "y", x, y, result);
}

bool Divide(double x, double y, double* result) {
  return Calc("divide", "numerator", "denominator", x, y, result);
}
```

## 局限

当然，cSoap 有很多局限，比如：
- 只支持 `int`, `double`, `bool` 和 `string` 这几种参数类型；
- 只支持 UTF-8 编码的消息内容；
- 一次调用一个连接；
- 连接是同步（阻塞）模式，可以指定 timeout（缺省为 15s）。

## 依赖

在实现上，cSoap 有下面这些依赖：
- Boost 1.66+；
- XML 解析和构造基于 PugiXml；
- 构建系统是 CMake，应该可以很方便地集成到其他 C++ 项目中。
