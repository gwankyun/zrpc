#include <iostream>
#include <string>
//#include "../include/zrpc.hpp"
#include <zrpc.hpp>

int main()
{
    try
    {
        asio::io_context io_context;

        auto socket = std::make_shared<asio::ip::tcp::socket>(io_context);
        zrpc::Client client(socket);
        client.connect("127.0.0.1", 12345);

        LOG(info, "1 + 1 = {}", client.call<int>("add", 1, 1));
        LOG(info, "1 + 9 = {}", client.call<int>("add", 1, 9));

        io_context.run();
    }
    catch (const std::exception& e)
    {
        LOG(info, e.what());
    }

    return 0;
}
