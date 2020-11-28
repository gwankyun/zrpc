#include <iostream>
#include <string>
#include <memory>
//#define ZRPC_HAS_CXX_11 1
#define ZRPC_DEBUG 1
#include <zrpc.hpp>
#include <dbg.h>

int count = 0;

void onResult(asio::error_code error, int& result, ZRPC_SHARED_PTR<zrpc::Client<asio::ip::tcp>> client)
{
    if (error)
    {
        return;
        dbg(error.message());
        count++;
        if (count > 1)
        {
            return;
        }
        client->asyncCall<int>("add", [client](asio::error_code e, int& result)
            {
                onResult(e, result, client);
            }, 0, 0);
        return;
    }
    dbg(result);
}

void onConnect(asio::error_code error, ZRPC_SHARED_PTR<zrpc::Client<asio::ip::tcp>> client)
{
    if (error)
    {
        dbg(error.message());
        return;
    }
    dbg("connect");
    client->asyncCall<int>("add", [client](asio::error_code e, int& result)
        {
            onResult(e, result, client);
        //}, std::string("yes"));
        }, 0, 0);
};

int main()
{
    asio::io_context io_context;

    auto client = ZRPC_MAKE_SHARED<zrpc::Client<asio::ip::tcp>>(io_context);

    client->asyncConnect("127.0.0.1", 3344, [client](asio::error_code error)
        {
            onConnect(error, client);
        });

    io_context.run();

    return 0;
}
