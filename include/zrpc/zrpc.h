#pragma once
#include <asio.hpp>
#include <apply.hpp>

#ifndef ZRPC_HAS_CXX_11
#  if __cplusplus >= 201103L || (defined(__cpp_variadic_templates) && defined(__cpp_rvalue_references))
#    define ZRPC_HAS_CXX_11 1
#  else
#    define ZRPC_HAS_CXX_11 0
#  endif // if __cplusplus >= 201103L || (defined(__cpp_variadic_templates) && defined(__cpp_rvalue_references))
#endif // !ZRPC_HAS_CXX_11

#if ZRPC_HAS_CXX_17

#endif // ZRPC_HAS_CXX_17

#if !ZRPC_HAS_CXX_11
#  include <boost/preprocessor.hpp>
#  include <boost/function.hpp>
#endif // !ZRPC_HAS_CXX_11

namespace zrpc
{
    typedef unsigned __int32 uint32_t;
    typedef unsigned __int16 uint16_t;

    //typedef asio::io_context io_context;

    namespace ip
    {
        typedef asio::ip::tcp tcp;
    }
}

