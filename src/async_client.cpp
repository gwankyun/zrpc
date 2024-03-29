#include <iostream>
#include <string>
#include <memory>
//#define ZRPC_HAS_CXX_11 1
#define ZRPC_DEBUG 1
#include <zrpc/client.hpp>
//#include <dbg.h>

#if ZRPC_USE_BOOST_ASIO
#  include <boost/asio.hpp>
namespace asio = boost::asio;
#else
#  include <asio.hpp>
#endif

int count = 0;

using ClientType = zrpc::Client< asio::ip::tcp, asio::io_context >;

void onResult(zrpc::error_code error, int& result, typename zrpc::shared_ptr<ClientType>::type client)
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
        client->asyncCall<int>("add", [client](zrpc::error_code e, int& result)
            {
                onResult(e, result, client);
            }, 0, 0);
        return;
    }
    //dbg(result);
    std::cout << result << std::endl;
}

void onConnect(zrpc::error_code error, typename zrpc::shared_ptr<ClientType>::type client)
{
    if (error)
    {
        //dbg(error.message());
        std::cout << error.message() << std::endl;
        return;
    }
    //dbg("connect");
    std::cout << "connect" << std::endl;
    client->asyncCall<int>("add", [client](zrpc::error_code e, int& result)
        {
            onResult(e, result, client);
        //}, std::string("yes"));
        }, 1, 9);
};

int main()
{
    asio::io_context io_context;

    auto client = ZRPC_MAKE_SHARED<ClientType>(io_context);

    client->asyncConnect("127.0.0.1", 3344, [client](zrpc::error_code error)
        {
            onConnect(error, client);
        });

    io_context.run();

    return 0;
}
