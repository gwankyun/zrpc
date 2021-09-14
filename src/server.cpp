#include <iostream>
#include <string>
#include <memory>
//#define ZRPC_HAS_CXX_11 0
#define ZRPC_DEBUG 1
#include <zrpc.hpp>
//#include <dbg.h>

int f1(int a)
{
    return a;
}

int f2(int a, int b)
{
    return a + b;
}

int f3(int a, int b, int c)
{
    return a + b + c;
}

int f4(int a, int b, int c, int d)
{
    return a + b + c + d;
}

int f5(int a, int b, int c, int d, int e)
{
    return a + b + c + d + e;
}

int f6(int a, int b, int c, int d, int e, int f)
{
    return a + b + c + d + e + f;
}

int f7(int a, int b, int c, int d, int e, int f, int g)
{
    return a + b + c + d + e + f + g;
}

int f8(int a, int b, int c, int d, int e, int f, int g, int h)
{
    return a + b + c + d + e + f + g + h;
}

int f9(int a, int b, int c, int d, int e, int f, int g, int h, int i)
{
    return a + b + c + d + e + f + g + h + i;
}

int add(int a, int b)
{
    //LOG(info, "a: {} b: {}", a, b);
    std::cout << "a: " << a << " b: " << b << std::endl;

    int result = a + b;
    //LOG(info, "result: {}", result);
    std::cout << "result: " << result << std::endl;

    return a + b;
}

std::vector<char> getVector(int n)
{
    dbg(n);
    std::vector<char> vec(n, '\0');
    return vec;
}

int main()
{
    //spdlog::easy::initialize(20, 30);
    LOG(info, "server");
    asio::io_context io_context;

    {
        zrpc::Server<asio::ip::tcp> server(io_context, asio::ip::tcp::v4(), 3344);
        //ZRPC_SHARED_PTR< zrpc::Server<asio::ip::tcp> > server(ZRPC_MAKE_SHARED< zrpc::Server<asio::ip::tcp> >(io_context, asio::ip::tcp::v4(), 3344));
        server.setTimeout(3 * 1000);
        server.bind("add", add);
        server.bind("getVector", getVector);
        server.bind("f1", f1);
        server.bind("f2", f2);
        server.bind("f3", f3);
        server.bind("f4", f4);
        server.bind("f5", f5);
        server.bind("f6", f6);
        server.bind("f7", f7);
        server.bind("f8", f8);
        server.bind("f9", f9);
        try
        {
            server();
        }
        catch (const std::exception& e)
        {
            dbg(e.what());
        }
    }

    io_context.run();
    return 0;
}
