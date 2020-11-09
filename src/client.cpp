#include <iostream>
#include <string>
#include <memory>
#define ZRPC_HAS_CXX_11 0
#define ZRPC_SHARED_PTR std::shared_ptr
#include <zrpc.hpp>

int main()
{
    //spdlog::easy::initialize();
    LOG(info, "client");
    try
    {
        zrpc::io_context io_context;

        zrpc::Client<zrpc::ip::tcp> client(io_context);
        client.connect("127.0.0.1", 3344);

        LOG(info, "1 + 1 = {}", client.call<int>("add", 1, 1));
        LOG(info, "1 + 9 = {}", client.call<int>("add", 1, 9));
    }
    catch (const std::exception& e)
    {
        LOG(info, e.what());
    }

    return 0;
}
