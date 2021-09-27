#pragma once

#if ZRPC_USE_BOOST_ASIO
#  include <boost/asio/coroutine.hpp>
#else
#  include <asio/coroutine.hpp>
#endif

#ifndef ZRPC_CORO_REENTER
#  if ZRPC_USE_BOOST_ASIO
#    define ZRPC_CORO_REENTER BOOST_ASIO_CORO_REENTER
#  else
#    define ZRPC_CORO_REENTER ASIO_CORO_REENTER
#  endif
#endif

#ifndef ZRPC_CORO_YIELD
#  if ZRPC_USE_BOOST_ASIO
#    define ZRPC_CORO_YIELD BOOST_ASIO_CORO_YIELD
#  else
#    define ZRPC_CORO_YIELD ASIO_CORO_YIELD
#  endif
#endif

#ifndef ZRPC_CORO_FORK
#  if ZRPC_USE_BOOST_ASIO
#    define ZRPC_CORO_FORK BOOST_ASIO_CORO_FORK
#  else
#    define ZRPC_CORO_FORK ASIO_CORO_FORK
#  endif
#endif
