#pragma once
#include "zrpc.h"
#include <boost/algorithm/hex.hpp>

#if ZRPC_CXX_STD_11
#include <log.hpp>
#else
#if !defined(LOG) && !defined(LOG_IF)
#define LOG(...)
#define LOG_IF(...)
#endif
#endif

#if ZRPC_HAS_CXX_11
#  include <functional>
#  include <memory>
#  include <vector>
#else
#  include <boost/make_shared.hpp>
#endif

#include <msgpack.hpp>

namespace zrpc
{
    namespace detail
    {
        typedef unsigned __int32 uint32_t;
        typedef unsigned __int16 uint16_t;

        using msgpack::type::tuple;

        struct Header
        {
            char sign[4];
            uint32_t length;
            uint16_t major;
            uint16_t minor;
        };

        template<typename T>
        inline void hex(const Header& header, T result)
        {
            boost::algorithm::hex(
                (const char*)(&header),
                (const char*)(&header) + sizeof(header),
                result);
        }

        inline std::string hex(const std::string& buffer)
        {
            std::string result;
            boost::algorithm::hex(
                buffer.begin(),
                buffer.end(),
                std::back_inserter(result));
            return result;
        }

        inline void initialize(Header& header)
        {
            memset(header.sign, '@', sizeof(header.sign));
            header.length = 0;
            header.major = 0;
            header.minor = 0;
        }

        struct Call
        {
            std::string func;
            std::string args;
            ~Call()
            {
                LOG(info, "");
            }
            MSGPACK_DEFINE(func, args);
        };

#if ZRPC_HAS_CXX_11
        using std::vector;
#else
        template<typename T>
        class vector
        {
        public:
            vector()
                : data_(NULL)
            {
            }

            vector(std::size_t size, T val)
                : data_(new T[size])
                , size_(size)
            {
                std::fill(data_, data_ + size, val);
            }

            ~vector()
            {
                if (data_ != NULL)
                {
                    delete[] data_;
                    data_ = NULL;
                }
            }

            T* data()
            {
                return data_;
            }

            std::size_t size()
            {
                return size_;
            }

        private:
            T* data_;
            std::size_t size_;
        };
#endif

        struct ICallable
        {
            ICallable()
            {
            }

            virtual ~ICallable()
            {
            }

            virtual std::string call(std::string param) = 0;
        };

#if ZRPC_HAS_CXX_11
        template<typename R, typename... Args>
        struct Callable : public ICallable
        {
            Callable(std::function<R(Args...)> func_)
                : func(func_)
            {
            }

            ~Callable()
            {
            }

            std::string call(std::string param) override
            {
                tuple<Args...> args;
                bool r = tryUnpack(param.c_str(), param.size(), args);
                if (!r)
                {
                    return "";
                }
                R result = lite::apply(func, args);
                return pack(result);
            }

            std::function<R(Args...)> func;
        };

        template<typename R, typename... Args>
        std::shared_ptr<ICallable> makeCallable(std::function<R(Args...)> func_)
        {
            return std::make_shared<Callable<R, Args...>>(func_);
        }

        template<typename R, typename... Args>
        std::shared_ptr<ICallable> makeCallable(R(*func_)(Args...))
        {
            return std::make_shared<Callable<R, Args...>>(func_);
        }
#else
        template<typename R, typename T0>
        struct Callable1 : public ICallable
        {
            Callable1(ZRPC_FUNCTION<R(T0)> func_)
                : func(func_)
            {
            }
            ~Callable1()
            {
            }
            std::string call(std::string param)
            {
                tuple<T0> args;
                bool u = tryUnpack(param.c_str(), param.size(), args);
                if (!u)
                {
                    return "";
                }
                R result = lite::apply(func, args);
                return pack(result);
            }
            ZRPC_FUNCTION<R(T0)> func;
        };

#ifndef ZRPC_CALLABLE
#  define ZRPC_CALLABLE(z, n, _) \
        template<typename R, BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPENAME, T)> \
        struct BOOST_PP_CAT(Callable, n) : public ICallable \
        { \
            BOOST_PP_CAT(Callable, n)(ZRPC_FUNCTION<R(BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T))> func_) \
                : func(func_) \
            { \
            } \
            ~BOOST_PP_CAT(Callable, n)() \
            { \
            } \
            std::string call(std::string param) \
            { \
                tuple<BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)> args; \
                bool u = tryUnpack(param.c_str(), param.size(), args); \
                if (!u) \
                { \
                    return ""; \
                } \
                R result = lite::apply(func, args); \
                return pack(result); \
            } \
            ZRPC_FUNCTION<R(BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T))> func; \
        };
        BOOST_PP_REPEAT_FROM_TO(2, 10, ZRPC_CALLABLE, _)
#endif

        template<typename R, typename T0>
        inline typename shared_ptr<ICallable>::type makeCallable(ZRPC_FUNCTION < R(T0) > func_)
        {
            return ZRPC_MAKE_SHARED< Callable1<R, T0> >(func_);
        }

        template<typename R, typename T0>
        inline typename shared_ptr<ICallable>::type makeCallable(R(*func_)(T0))
        {
            return ZRPC_MAKE_SHARED< Callable1<R, T0> >(func_);
        }

#ifndef ZRPC_MAKE_CALLABLE
#  define ZRPC_MAKE_CALLABLE(z, n, _) \
        template<typename R, BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPENAME, T)> \
        inline typename shared_ptr<ICallable>::type makeCallable(ZRPC_FUNCTION < R(BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)) > func_) \
        { \
            return ZRPC_MAKE_SHARED< BOOST_PP_CAT(Callable, n)<R, BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)> >(func_); \
        } \
        template<typename R, BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPENAME, T)> \
        inline typename shared_ptr<ICallable>::type makeCallable(R(*func_)(BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T))) \
        { \
            return ZRPC_MAKE_SHARED< BOOST_PP_CAT(Callable, n)<R, BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)> >(func_); \
        }
        BOOST_PP_REPEAT_FROM_TO(2, 10, ZRPC_MAKE_CALLABLE, _)
#endif
#endif
    }
}
