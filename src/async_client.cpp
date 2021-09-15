#include <iostream>
#include <string>
#include <memory>
//#define ZRPC_HAS_CXX_11 1
#define ZRPC_DEBUG 1
#include <zrpc.hpp>
//#include <dbg.h>

int count = 0;

void onResult(asio::error_code error, int& result, typename zrpc::shared_ptr<zrpc::Client<asio::ip::tcp>>::type client)
{
    if (error)
    {
        return;
        //dbg(error.message());
        std::cout << error.message() << std::endl;
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
    //dbg(result);
    std::cout << result << std::endl;
}

void onConnect(asio::error_code error, typename zrpc::shared_ptr<zrpc::Client<asio::ip::tcp>>::type client)
{
    if (error)
    {
        //dbg(error.message());
        std::cout << error.message() << std::endl;
        return;
    }
    //dbg("connect");
    std::cout << "connect" << std::endl;
    client->asyncCall<int>("add", [client](asio::error_code e, int& result)
        {
            onResult(e, result, client);
        //}, std::string("yes"));
        }, 1, 9);
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
