#pragma once
#include <apply.hpp>

//#define ZRPC_HAS_CXX_11 0
#ifndef ZRPC_HAS_CXX_11
#  if __cplusplus >= 201103L || (defined(__cpp_variadic_templates) && defined(__cpp_rvalue_references))
#    define ZRPC_HAS_CXX_11 1
#  else
#    define ZRPC_HAS_CXX_11 0
#  endif
#endif

#ifndef ZRPC_HAS_CXX_17
#  if ZRPC_HAS_CXX_11 && (__cplusplus >= 201703L || defined(__cpp_lib_optional))
#    define ZRPC_HAS_CXX_17 1
#  else
#    define ZRPC_HAS_CXX_17 0
#  endif
#endif

#if !ZRPC_HAS_CXX_11
#  include <boost/preprocessor.hpp>
#  include <boost/function.hpp>
#endif

#if ZRPC_HAS_CXX_17
//#  include <dbg.h>
#endif // ZRPC_HAS_CXX_17

#define dbg(...)

#ifndef ZRPC_DEBUG
#  define ZRPC_DEBUG 0
#endif // !ZRPC_DEBUG

//#if defined(dbg)
//#  define dgbIf(condition, ...) \
//    if (condition) \
//    { \
//        dbg(##__VA_ARGS__); \
//    }
//#  ifndef zdbg
//#    if ZRPC_DEBUG
//#      define zdbg(...) dbg(##__VA_ARGS__)
//#    else
//#      define zdbg(...)
//#    endif
//#  endif // !zdbg
//#endif // defined(dbg)

#ifndef zdbg
#  if ZRPC_DEBUG && defined(dbg)
#    define zdbg(...) dbg(##__VA_ARGS__)
#  else
#    define zdbg(x) std::cout << __func__ << ":" << __LINE__ << " " << x << std::endl
#  endif // ZRPC_DEBUG && defined(dbg)
#endif // !zdbg

#define zdbg(...) dbg(##__VA_ARGS__)

#ifndef ZRPC_USE_BOOST_SHARED_PTR
#  define ZRPC_USE_BOOST_SHARED_PTR 0
#endif // !ZRPC_USE_BOOST_SHARED_PTR

#if !defined(ZRPC_SHARED_PTR) && !defined(ZRPC_MAKE_SHARED)
#  if ZRPC_HAS_CXX_11 && !ZRPC_USE_BOOST_SHARED_PTR
//#    define ZRPC_SHARED_PTR std::shared_ptr
#    define ZRPC_MAKE_SHARED std::make_shared
#  else
//#    define ZRPC_SHARED_PTR boost::shared_ptr
#    define ZRPC_MAKE_SHARED boost::make_shared
#  endif
#endif

#ifndef ZRPC_USE_BOOST_ENABLE_SHARED_FROM_THIS
#  define ZRPC_USE_BOOST_ENABLE_SHARED_FROM_THIS 0
#endif

#ifndef ZRPC_ENABLE_SHARED_FROM_THIS
#  if ZRPC_HAS_CXX_11 && !ZRPC_USE_BOOST_ENABLE_SHARED_FROM_THIS
#    define ZRPC_ENABLE_SHARED_FROM_THIS std::enable_shared_from_this
#    undef ZRPC_USE_BOOST_ENABLE_SHARED_FROM_THIS
#    define ZRPC_USE_BOOST_ENABLE_SHARED_FROM_THIS 0
#  else
#    define ZRPC_ENABLE_SHARED_FROM_THIS boost::enable_shared_from_this
#    undef ZRPC_USE_BOOST_ENABLE_SHARED_FROM_THIS
#    define ZRPC_USE_BOOST_ENABLE_SHARED_FROM_THIS 1
#  endif
#endif

#ifndef ZRPC_USE_BOOST_FUNCTION
#  define ZRPC_USE_BOOST_FUNCTION 0
#endif

#ifndef ZRPC_FUNCTION
#  if ZRPC_HAS_CXX_11 && !ZRPC_USE_BOOST_FUNCTION
#    define ZRPC_FUNCTION std::function
#    undef ZRPC_USE_BOOST_FUNCTION
#    define ZRPC_USE_BOOST_FUNCTION 0
#  else
#    define ZRPC_FUNCTION boost::function
#    undef ZRPC_USE_BOOST_FUNCTION
#    define ZRPC_USE_BOOST_FUNCTION 1
#  endif
#endif

#ifndef ZRPC_HAS_CONCEPTS
#  if defined(__cpp_concepts)
#    define ZRPC_HAS_CONCEPTS 1
#  else
#    define ZRPC_HAS_CONCEPTS 0
#  endif
#endif

namespace zrpc
{
    typedef unsigned __int32 uint32_t;
    typedef unsigned __int16 uint16_t;

    template<typename T>
    struct optional
    {
#if ZRPC_HAS_CXX_17 && !ZRPC_USE_BOOST_OPTIONAL
        typedef std::optional<T> type;
#else
        typedef boost::optional<T> type;
#endif
    };

#if ZRPC_HAS_CXX_17 && !ZRPC_USE_BOOST_OPTIONAL
    typedef std::nullopt_t nullopt_t;
#else
    typedef boost::none_t nullopt_t;
#endif

    inline nullopt_t nullopt()
    {
#if ZRPC_HAS_CXX_17 && !ZRPC_USE_BOOST_OPTIONAL
        return std::nullopt;
#else
        return boost::none;
#endif
    }

    template<typename T>
    struct shared_ptr
    {
#if ZRPC_HAS_CXX_11
        typedef std::shared_ptr<T> type;
#else
        typedef boost::shared_ptr<T> type;
#endif
    };

//    template<typename T>
//    struct enable_shared_from_this
//    {
//#if ZRPC_HAS_CXX_11
//        typedef std::enable_shared_from_this<T> type;
//#else
//        typedef boost::enable_shared_from_this<T> type;
//#endif
//    };
}
