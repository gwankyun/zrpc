#include <iostream>
#include <string>
#include <memory>
#define ZRPC_HAS_CXX_11 1
#include <zrpc.hpp>

int main()
{
    LOG(info, "client");
    try
    {
        asio::io_context io_context;

        zrpc::Client<asio::ip::tcp> client(io_context);
        client.connect("127.0.0.1", 3344);

#if ZRPC_HAS_CXX_11
        LOG(info, "1 + 1 = {}", client.call<int>("add", 1, 1));
        LOG(info, "1 + 9 = {}", client.call<int>("add", 1, 9));
        std::cout << "1 + 1 = " << client.call<int>("add", 1, 1) << std::endl;
        std::cout << "1 + 9 = " << client.call<int>("add", 0, 9) << std::endl;
#else
        std::cout << "1 + 1 = " << client.call<int>("add", 1, 1) << std::endl;
        std::cout << "1 + 9 = " << client.call<int>("add", 0, 9) << std::endl;
#endif // ZRPC_HAS_CXX_11
    }
    catch (const std::exception& e)
    {
        LOG(info, e.what());
    }

    return 0;
}
