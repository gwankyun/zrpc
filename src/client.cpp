#include <iostream>
#include <string>
#include <memory>
#include <vector>
//#define ZRPC_HAS_CXX_11 0
#include <zrpc.hpp>
//#include <dbg.h>
#include <boost/optional/optional_io.hpp>

int main()
{
    asio::io_context io_context;

    zrpc::Client<asio::ip::tcp> client(io_context);
    client.connect("127.0.0.1", 3344);

    auto error = client.tryCall<int>("add", "1");
    //auto right = client.tryCall<int>("add", 1, 9);

    //dbg(error);
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
        //dbg(e.what());
        std::cout << e.what() << std::endl;
    }

    //while (true)
    //{
    //    Sleep(3 * 1000);
    //}

    std::cout << "f1: " << client.call<int>("f1", 1) << std::endl;
    std::cout << "f2: " << client.call<int>("f2", 1, 2) << std::endl;
    std::cout << "f3: " << client.call<int>("f3", 1, 2, 3) << std::endl;
    std::cout << "f4: " << client.call<int>("f4", 1, 2, 3, 4) << std::endl;
    std::cout << "f5: " << client.call<int>("f5", 1, 2, 3, 4, 5) << std::endl;
    std::cout << "f6: " << client.call<int>("f6", 1, 2, 3, 4, 5, 6) << std::endl;
    std::cout << "f7: " << client.call<int>("f7", 1, 2, 3, 4, 5, 6, 7) << std::endl;
    std::cout << "f8: " << client.call<int>("f8", 1, 2, 3, 4, 5, 6, 7, 8) << std::endl;
    std::cout << "f9: " << client.call<int>("f9", 1, 2, 3, 4, 5, 6, 7, 8, 9) << std::endl;

    return 0;
}
