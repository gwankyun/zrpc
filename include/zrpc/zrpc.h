#pragma once
#include <asio.hpp>

#define EXTEND1(arg_, fn_, s_) fn_(arg_, 1)
#define EXTEND2(arg_, fn_, s_) s_(fn_(arg_, 1)) fn_(arg_, 2)
#define EXTEND3(arg_, fn_, s_) s_(fn_(arg_, 1)) s_(fn_(arg_, 2)) fn_(arg_, 3)
#define EXTEND4(arg_, fn_, s_) s_(fn_(arg_, 1)) s_(fn_(arg_, 2)) s_(fn_(arg_, 3)) fn_(arg_, 4)
#define EXTEND5(arg_, fn_, s_) s_(fn_(arg_, 1)) s_(fn_(arg_, 2)) s_(fn_(arg_, 3)) s_(fn_(arg_, 4)) fn_(arg_, 5)
#define EXTEND6(arg_, fn_, s_) s_(fn_(arg_, 1)) s_(fn_(arg_, 2)) s_(fn_(arg_, 3)) s_(fn_(arg_, 4)) s_(fn_(arg_, 5)) fn_(arg_, 6)
#define EXTEND7(arg_, fn_, s_) s_(fn_(arg_, 1)) s_(fn_(arg_, 2)) s_(fn_(arg_, 3)) s_(fn_(arg_, 4)) s_(fn_(arg_, 5)) s_(fn_(arg_, 6)) fn_(arg_, 7)
#define EXTEND8(arg_, fn_, s_) s_(fn_(arg_, 1)) s_(fn_(arg_, 2)) s_(fn_(arg_, 3)) s_(fn_(arg_, 4)) s_(fn_(arg_, 5)) s_(fn_(arg_, 6)) s_(fn_(arg_, 7)) fn_(arg_, 8)
#define EXTEND9(arg_, fn_, s_) s_(fn_(arg_, 1)) s_(fn_(arg_, 2)) s_(fn_(arg_, 3)) s_(fn_(arg_, 4)) s_(fn_(arg_, 5)) s_(fn_(arg_, 6)) s_(fn_(arg_, 7)) s_(fn_(arg_, 8)) fn_(arg_, 9)

#define EXTEND(arg_, fn_, n_, s_) EXTEND##n_(arg_, fn_, s_)

#define COMMA(x) x,
#define SEMICOLON(x) x;

#define CALL(x, n) x##n

#define ARG(x, n) const x##n& _##x##n

#define REF_ARG(x, n) x##n& _##x##n

#define TYPENAME(x, n) typename x##n

#define DEFINE(arg_) \
    arg_(1) \
    \
    arg_(2) \
    \
    arg_(3) \
    \
    arg_(4) \
    \
    arg_(5) \
    \
    arg_(6) \
    \
    arg_(7) \
    \
    arg_(8) \
    \
    arg_(9)

namespace zrpc
{
    typedef unsigned __int32 uint32_t;
    typedef unsigned __int16 uint16_t;

    typedef asio::io_context io_context;

    namespace ip
    {
        typedef asio::ip::tcp tcp;
    }
}

#ifndef ZRPC_HAS_CXX_11
#  if defined(__cpp_variadic_templates) && defined(__cpp_rvalue_references)
#    define ZRPC_HAS_CXX_11 1
#  else
#    define ZRPC_HAS_CXX_11 0
#  endif // defined(__cpp_variadic_templates) && defined(__cpp_rvalue_references)
#endif // !ZRPC_HAS_CXX_11

