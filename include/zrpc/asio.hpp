#pragma once

#if ZRPC_USE_BOOST_ASIO
#  include <boost/system/error_code.hpp>
#  include <boost/asio.hpp>
#else
#  include <asio/error_code.hpp>
#  include <asio/buffer.hpp>
#  include <asio/steady_timer.hpp>
#  include <asio/detail/chrono.hpp>
#  include <asio/error_code.hpp>
#  include <asio/buffer.hpp>
#  include <asio/steady_timer.hpp>
#  include <asio/detail/chrono.hpp>
#endif

namespace zrpc
{
    namespace detail
    {
#if ZRPC_USE_BOOST_ASIO
        using boost::system::error_code;
        using namespace boost::asio;
#else
        using namespace asio;
        typedef asio::ASIO_CONST_BUFFER const_buffer;
        typedef asio::ASIO_MUTABLE_BUFFER mutable_buffer;
#endif
    }
    using detail::error_code;

#if ZRPC_HAS_CONCEPTS
    template<typename T>
    concept Contextable = requires (T context)
    {
        context.run();
    };

    template<typename T>
    concept Protocolable = requires
    {
        typename T::socket;
        typename T::acceptor;
        typename T::endpoint;
    };
#endif
}
