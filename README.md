# zrpc

跨平臺輕量級遠程調用框架

- 無須編譯，兼容C++98。
- 網絡層使用asio，序列化使用msgpack。
- 客戶端支持異步調用。

## 示例

### 服務端

```C++
#include <iostream>
#include <zrpc/server.hpp>

#if ZRPC_USE_BOOST_ASIO
#  include <boost/asio.hpp>
namespace asio = boost::asio;
#else
#  include <asio.hpp>
#endif

int add(int a, int b)
{
    return a + b;
}

int main()
{
    asio::io_context io_context;

    zrpc::Server<asio::io_context, asio::ip::tcp> server(io_context, asio::ip::tcp::v4(), 12345);

    server.bind("add", add);

    try
    {
        server();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    io_context.run();
    return 0;
}
```

### 客戶端

```C++
#include <iostream>
#include <zrpc/client.hpp>

#if ZRPC_USE_BOOST_ASIO
#  include <boost/asio.hpp>
namespace asio = boost::asio;
#else
#  include <asio.hpp>
#endif

int main()
{
    asio::io_context io_context;

    zrpc::Client<asio::io_context, asio::ip::tcp> client(io_context);
    client.connect("127.0.0.1", 12345);

    std::cout << "1 + 2 = " << client.call<int>("add", 1, 2) << std::endl;

    return 0;
}
```
