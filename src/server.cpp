#include <iostream>
#include <string>
#include <zrpc.hpp>

std::string add(std::string buffer)
{
    int a;
    int b;
    zrpc::unpack(buffer, a, b);
    LOG(info, "a: {} b: {}", a, b);

    int result = a + b;
    LOG(info, "result: {}", result);

    return msgpack::easy::pack(result);
}

int main()
{
    asio::io_context io_context;

    asio::ip::tcp::acceptor acceptor(
        io_context,
        asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 12345));

    {
        auto socket = std::make_shared<asio::ip::tcp::socket>(io_context);
        zrpc::Server server(acceptor, socket);
        server.bind("add", add);
        server();
    }

    io_context.run();
    return 0;
}
