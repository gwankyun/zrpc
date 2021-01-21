#include <iostream>
#include <string>
#include <memory>
#include <vector>
//#define ZRPC_HAS_CXX_11 0
#include <zrpc.hpp>
#include <dbg.h>
#include <boost/optional/optional_io.hpp>

int main()
{
    asio::io_context io_context;

    zrpc::Client<asio::ip::tcp> client(io_context);
    client.connect("127.0.0.1", 3344);

    auto error = client.tryCall<int>("add", "1");
    auto right = client.tryCall<int>("add", 1, 9);

    dbg(error);
    dbg(right);

    dbg(client.call<int>("add", 1, 9));

    try
    {
        //dbg(client.call<int>("add", "1"));
        auto vec = client.tryCall<std::vector<char>>("getVector", 5);
        if (vec)
        {
            dbg(*vec);
        }
    }
    catch (const std::exception& e)
    {
        dbg(e.what());
    }

    while (true)
    {
        Sleep(3 * 1000);
    }

    return 0;
}
