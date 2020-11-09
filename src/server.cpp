#include <iostream>
#include <string>
#include <memory>
#define ZRPC_HAS_CXX_11 0
#define ZRPC_SHARED_PTR std::shared_ptr
#include <zrpc.hpp>

//std::string add(std::string buffer)
//{
//    int a;
//    int b;
//    zrpc::unpack(buffer, a, b);
//    LOG(info, "a: {} b: {}", a, b);
//
//    int result = a + b;
//    LOG(info, "result: {}", result);
//
//    return zrpc::pack(result);
//}

int add(int a, int b)
{
    //int a;
    //int b;
    //zrpc::unpack(buffer, a, b);
    LOG(info, "a: {} b: {}", a, b);

    int result = a + b;
    LOG(info, "result: {}", result);

    return a + b;
}

int main()
{
    //spdlog::easy::initialize(20, 30);
    LOG(info, "server");
    zrpc::io_context io_context;
    //zrpc::ip::tcp::acceptor acceptor(io_context, zrpc::ip::tcp::endpoint(zrpc::ip::tcp::v4(), 12345));

    {
        zrpc::Server<zrpc::ip::tcp> server(io_context, zrpc::ip::tcp::v4(), 3344);
        //zrpc::Server<zrpc::ip::tcp> server(acceptor);
        server.bind("add", add);
        server();
    }

    io_context.run();
    return 0;
}
