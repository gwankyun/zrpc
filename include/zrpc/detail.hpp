#pragma once
#include "zrpc.h"
//#include <msgpack_easy.hpp>
#include <boost/algorithm/hex.hpp>

#if ZRPC_CXX_STD_11
#include <log.hpp>
#else
#if !defined(LOG) && !defined(LOG_IF)
#define LOG(...)
#define LOG_IF(...)
#endif
#endif // ZRPC_CXX_STD_11

namespace zrpc
{
    namespace detail
    {
        typedef unsigned __int32 uint32_t;
        typedef unsigned __int16 uint16_t;

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
            Callable(ZRPC_FUNCTION<R(Args...)> func_)
                : func(func_)
            {
            }

            ~Callable()
            {
            }

            std::string call(std::string param) override
            {
                msgpack::type::tuple<Args...> args;
                bool r = tryUnpack(param.c_str(), param.size(), args);
                if (!r)
                {
                    return "";
                }
                R result = lite::apply(func, args);
                return pack(result);
            }

            ZRPC_FUNCTION<R(Args...)> func;
        };

        template<typename R, typename... Args>
        ZRPC_SHARED_PTR<ICallable> makeCallable(std::function<R(Args...)> func_)
        {
            return ZRPC_MAKE_SHARED<Callable<R, Args...>>(func_);
        }

        template<typename R, typename... Args>
        ZRPC_SHARED_PTR<ICallable> makeCallable(R(*func_)(Args...))
        {
            return ZRPC_MAKE_SHARED<Callable<R, Args...>>(func_);
        }
#else
#  define ZRPC_CALLABLE(z, n, _) \
        template< \
            typename R BOOST_PP_COMMA_IF(n) \
            BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPENAME, T)> \
        struct BOOST_PP_CAT(Callable, n) : public ICallable \
        { \
            BOOST_PP_CAT(Callable, n)( \
                ZRPC_FUNCTION<R(BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T))> func_) \
                : func(func_) \
            { \
            } \
            ~BOOST_PP_CAT(Callable, n)() \
            { \
            } \
            std::string call(std::string param) \
            { \
                msgpack::type::tuple<BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)> args; \
                bool u = tryUnpack(param.c_str(), parm.size(), args); \
                if (!u) \
                { \
                    return ""; \
                } \
                R result = lite::apply(func, args); \
                return pack(result); \
            } \
            ZRPC_FUNCTION<R(BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T))> func; \
        }; \
        template< \
            typename R BOOST_PP_COMMA_IF(n) \
            BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPENAME, T)> \
        ZRPC_SHARED_PTR<ICallable> makeCallable(ZRPC_FUNCTION<R(BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T))> func_) \
        { \
            return ZRPC_MAKE_SHARED<BOOST_PP_CAT(Callable, n)<R BOOST_PP_COMMA_IF(n) BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)> >(func_); \
        } \
        template<typename R BOOST_PP_COMMA_IF(n) BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPENAME, T)> \
        ZRPC_SHARED_PTR<ICallable> makeCallable(R(*func_)(BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T))) \
        { \
            return ZRPC_MAKE_SHARED<BOOST_PP_CAT(Callable, n)<R BOOST_PP_COMMA_IF(n) BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)> >(func_); \
        }

        BOOST_PP_REPEAT(10, ZRPC_CALLABLE, _)
#endif // ZRPC_HAS_CXX_11
    }
}
