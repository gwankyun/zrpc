#include <benchmark/benchmark.h>
#include <zrpc/client.hpp>
#include <boost/optional/optional_io.hpp>

#if ZRPC_USE_BOOST_ASIO
#  include <boost/asio.hpp>
namespace asio = boost::asio;
#else
#  include <asio.hpp>
#endif

static void BM_zrpc(benchmark::State& state)
{
    asio::io_context io_context;

    zrpc::Client<asio::io_context, asio::ip::tcp> client(io_context);
    client.connect("127.0.0.1", 3344);

    for (auto _ : state)
    {
        int result = client.call<int>("f1", 1);
    }
}
BENCHMARK(BM_zrpc);

BENCHMARK_MAIN();
