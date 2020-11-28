#include <iostream>
#include <string>
#include <memory>
//#define ZRPC_HAS_CXX_11 0
#define ZRPC_DEBUG 1
#include <zrpc.hpp>
#include <dbg.h>

int add(int a, int b)
{
    LOG(info, "a: {} b: {}", a, b);

    int result = a + b;
    LOG(info, "result: {}", result);

    return a + b;
}

int main()
{
    //spdlog::easy::initialize(20, 30);
    LOG(info, "server");
    asio::io_context io_context;

    {
        zrpc::Server<asio::ip::tcp> server(io_context, asio::ip::tcp::v4(), 3344);
        server.bind("add", add);
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
