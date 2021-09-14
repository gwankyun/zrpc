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
                msgpack::type::tuple<T0> args;
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

        //template<typename R, BOOST_PP_REPEAT(2, BOOST_PP_TYPENAME, T)>
        //struct Callable2 : public ICallable
        //{
        //    Callable2(ZRPC_FUNCTION<R(BOOST_PP_REPEAT(2, BOOST_PP_TYPE, T))> func_)
        //        : func(func_)
        //    {
        //    }
        //    ~Callable2()
        //    {
        //    }
        //    std::string call(std::string param)
        //    {
        //        msgpack::type::tuple<BOOST_PP_REPEAT(2, BOOST_PP_TYPE, T)> args;
        //        bool u = tryUnpack(param.c_str(), param.size(), args);
        //        if (!u)
        //        {
        //            return "";
        //        }
        //        R result = lite::apply(func, args);
        //        return pack(result);
        //    }
        //    ZRPC_FUNCTION<R(BOOST_PP_REPEAT(2, BOOST_PP_TYPE, T))> func;
        //};

        //template<typename R, BOOST_PP_REPEAT(3, BOOST_PP_TYPENAME, T)>
        //struct Callable3 : public ICallable
        //{
        //    Callable3(ZRPC_FUNCTION<R(BOOST_PP_REPEAT(3, BOOST_PP_TYPE, T))> func_)
        //        : func(func_)
        //    {
        //    }
        //    ~Callable3()
        //    {
        //    }
        //    std::string call(std::string param)
        //    {
        //        msgpack::type::tuple<BOOST_PP_REPEAT(3, BOOST_PP_TYPE, T)> args;
        //        bool u = tryUnpack(param.c_str(), param.size(), args);
        //        if (!u)
        //        {
        //            return "";
        //        }
        //        R result = lite::apply(func, args);
        //        return pack(result);
        //    }
        //    ZRPC_FUNCTION<R(BOOST_PP_REPEAT(3, BOOST_PP_TYPE, T))> func;
        //};

        //template<typename R, BOOST_PP_REPEAT(4, BOOST_PP_TYPENAME, T)>
        //struct Callable4 : public ICallable
        //{
        //    Callable4(ZRPC_FUNCTION<R(BOOST_PP_REPEAT(4, BOOST_PP_TYPE, T))> func_)
        //        : func(func_)
        //    {
        //    }
        //    ~Callable4()
        //    {
        //    }
        //    std::string call(std::string param)
        //    {
        //        msgpack::type::tuple<BOOST_PP_REPEAT(4, BOOST_PP_TYPE, T)> args;
        //        bool u = tryUnpack(param.c_str(), param.size(), args);
        //        if (!u)
        //        {
        //            return "";
        //        }
        //        R result = lite::apply(func, args);
        //        return pack(result);
        //    }
        //    ZRPC_FUNCTION<R(BOOST_PP_REPEAT(4, BOOST_PP_TYPE, T))> func;
        //};

        //template<typename R, BOOST_PP_REPEAT(5, BOOST_PP_TYPENAME, T)>
        //struct Callable5 : public ICallable
        //{
        //    Callable5(ZRPC_FUNCTION<R(BOOST_PP_REPEAT(5, BOOST_PP_TYPE, T))> func_)
        //        : func(func_)
        //    {
        //    }
        //    ~Callable5()
        //    {
        //    }
        //    std::string call(std::string param)
        //    {
        //        msgpack::type::tuple<BOOST_PP_REPEAT(5, BOOST_PP_TYPE, T)> args;
        //        bool u = tryUnpack(param.c_str(), param.size(), args);
        //        if (!u)
        //        {
        //            return "";
        //        }
        //        R result = lite::apply(func, args);
        //        return pack(result);
        //    }
        //    ZRPC_FUNCTION<R(BOOST_PP_REPEAT(5, BOOST_PP_TYPE, T))> func;
        //};

        //template<typename R, BOOST_PP_REPEAT(6, BOOST_PP_TYPENAME, T)>
        //struct Callable6 : public ICallable
        //{
        //    Callable6(ZRPC_FUNCTION<R(BOOST_PP_REPEAT(6, BOOST_PP_TYPE, T))> func_)
        //        : func(func_)
        //    {
        //    }
        //    ~Callable6()
        //    {
        //    }
        //    std::string call(std::string param)
        //    {
        //        msgpack::type::tuple<BOOST_PP_REPEAT(6, BOOST_PP_TYPE, T)> args;
        //        bool u = tryUnpack(param.c_str(), param.size(), args);
        //        if (!u)
        //        {
        //            return "";
        //        }
        //        R result = lite::apply(func, args);
        //        return pack(result);
        //    }
        //    ZRPC_FUNCTION<R(BOOST_PP_REPEAT(6, BOOST_PP_TYPE, T))> func;
        //};

        //template<typename R, BOOST_PP_REPEAT(7, BOOST_PP_TYPENAME, T)>
        //struct Callable7 : public ICallable
        //{
        //    Callable7(ZRPC_FUNCTION<R(BOOST_PP_REPEAT(7, BOOST_PP_TYPE, T))> func_)
        //        : func(func_)
        //    {
        //    }
        //    ~Callable7()
        //    {
        //    }
        //    std::string call(std::string param)
        //    {
        //        msgpack::type::tuple<BOOST_PP_REPEAT(7, BOOST_PP_TYPE, T)> args;
        //        bool u = tryUnpack(param.c_str(), param.size(), args);
        //        if (!u)
        //        {
        //            return "";
        //        }
        //        R result = lite::apply(func, args);
        //        return pack(result);
        //    }
        //    ZRPC_FUNCTION<R(BOOST_PP_REPEAT(7, BOOST_PP_TYPE, T))> func;
        //};

        //template<typename R, BOOST_PP_REPEAT(8, BOOST_PP_TYPENAME, T)>
        //struct Callable8 : public ICallable
        //{
        //    Callable8(ZRPC_FUNCTION<R(BOOST_PP_REPEAT(8, BOOST_PP_TYPE, T))> func_)
        //        : func(func_)
        //    {
        //    }
        //    ~Callable8()
        //    {
        //    }
        //    std::string call(std::string param)
        //    {
        //        msgpack::type::tuple<BOOST_PP_REPEAT(8, BOOST_PP_TYPE, T)> args;
        //        bool u = tryUnpack(param.c_str(), param.size(), args);
        //        if (!u)
        //        {
        //            return "";
        //        }
        //        R result = lite::apply(func, args);
        //        return pack(result);
        //    }
        //    ZRPC_FUNCTION<R(BOOST_PP_REPEAT(8, BOOST_PP_TYPE, T))> func;
        //};

        //template<typename R, BOOST_PP_REPEAT(9, BOOST_PP_TYPENAME, T)>
        //struct Callable9 : public ICallable
        //{
        //    Callable9(ZRPC_FUNCTION<R(BOOST_PP_REPEAT(9, BOOST_PP_TYPE, T))> func_)
        //        : func(func_)
        //    {
        //    }
        //    ~Callable9()
        //    {
        //    }
        //    std::string call(std::string param)
        //    {
        //        msgpack::type::tuple<BOOST_PP_REPEAT(9, BOOST_PP_TYPE, T)> args;
        //        bool u = tryUnpack(param.c_str(), param.size(), args);
        //        if (!u)
        //        {
        //            return "";
        //        }
        //        R result = lite::apply(func, args);
        //        return pack(result);
        //    }
        //    ZRPC_FUNCTION<R(BOOST_PP_REPEAT(9, BOOST_PP_TYPE, T))> func;
        //};

#ifndef ZRPC_CALLABLE
#define ZRPC_CALLABLE(z, n, _) \
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
                msgpack::type::tuple<BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)> args; \
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

        //template< \
        //    typename R BOOST_PP_COMMA_IF(n) \
        //    BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPENAME, T)> \
        //ZRPC_SHARED_PTR<ICallable> makeCallable(ZRPC_FUNCTION<R(BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T))> func_) \
        //{ \
        //    return ZRPC_MAKE_SHARED<BOOST_PP_CAT(Callable, n)<R BOOST_PP_COMMA_IF(n) BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)> >(func_); \
        //} \
        //template<typename R BOOST_PP_COMMA_IF(n) BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPENAME, T)> \
        //ZRPC_SHARED_PTR<ICallable> makeCallable(R(*func_)(BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T))) \
        //{ \
        //    return ZRPC_MAKE_SHARED<BOOST_PP_CAT(Callable, n)<R BOOST_PP_COMMA_IF(n) BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)> >(func_); \
        //}

        //BOOST_PP_REPEAT(10, ZRPC_CALLABLE, _);

        template<typename R, typename T0>
        inline ZRPC_SHARED_PTR<ICallable> makeCallable(ZRPC_FUNCTION < R(T0) > func_)
        {
            return ZRPC_MAKE_SHARED< Callable1<R, T0> >(func_);
        }

        template<typename R, typename T0>
        inline ZRPC_SHARED_PTR<ICallable> makeCallable(R(*func_)(T0))
        {
            return ZRPC_MAKE_SHARED< Callable1<R, T0> >(func_);
        }

        //template<typename R, BOOST_PP_REPEAT(2, BOOST_PP_TYPENAME, T)>
        //inline ZRPC_SHARED_PTR<ICallable> makeCallable(ZRPC_FUNCTION < R(BOOST_PP_REPEAT(2, BOOST_PP_TYPE, T)) > func_)
        //{
        //    return ZRPC_MAKE_SHARED< Callable2<R, BOOST_PP_REPEAT(2, BOOST_PP_TYPE, T)> >(func_);
        //}

        //template<typename R, BOOST_PP_REPEAT(2, BOOST_PP_TYPENAME, T)>
        //inline ZRPC_SHARED_PTR<ICallable> makeCallable(R(*func_)(BOOST_PP_REPEAT(2, BOOST_PP_TYPE, T)))
        //{
        //    return ZRPC_MAKE_SHARED< Callable2<R, BOOST_PP_REPEAT(2, BOOST_PP_TYPE, T)> >(func_);
        //}

        //template<typename R, BOOST_PP_REPEAT(3, BOOST_PP_TYPENAME, T)>
        //inline ZRPC_SHARED_PTR<ICallable> makeCallable(ZRPC_FUNCTION < R(BOOST_PP_REPEAT(3, BOOST_PP_TYPE, T)) > func_)
        //{
        //    return ZRPC_MAKE_SHARED< Callable3<R, BOOST_PP_REPEAT(3, BOOST_PP_TYPE, T)> >(func_);
        //}

        //template<typename R, BOOST_PP_REPEAT(3, BOOST_PP_TYPENAME, T)>
        //inline ZRPC_SHARED_PTR<ICallable> makeCallable(R(*func_)(BOOST_PP_REPEAT(3, BOOST_PP_TYPE, T)))
        //{
        //    return ZRPC_MAKE_SHARED< Callable3<R, BOOST_PP_REPEAT(3, BOOST_PP_TYPE, T)> >(func_);
        //}

        //template<typename R, BOOST_PP_REPEAT(4, BOOST_PP_TYPENAME, T)>
        //inline ZRPC_SHARED_PTR<ICallable> makeCallable(ZRPC_FUNCTION < R(BOOST_PP_REPEAT(4, BOOST_PP_TYPE, T)) > func_)
        //{
        //    return ZRPC_MAKE_SHARED< Callable4<R, BOOST_PP_REPEAT(4, BOOST_PP_TYPE, T)> >(func_);
        //}

        //template<typename R, BOOST_PP_REPEAT(4, BOOST_PP_TYPENAME, T)>
        //inline ZRPC_SHARED_PTR<ICallable> makeCallable(R(*func_)(BOOST_PP_REPEAT(4, BOOST_PP_TYPE, T)))
        //{
        //    return ZRPC_MAKE_SHARED< Callable4<R, BOOST_PP_REPEAT(4, BOOST_PP_TYPE, T)> >(func_);
        //}

        //template<typename R, BOOST_PP_REPEAT(5, BOOST_PP_TYPENAME, T)>
        //inline ZRPC_SHARED_PTR<ICallable> makeCallable(ZRPC_FUNCTION < R(BOOST_PP_REPEAT(5, BOOST_PP_TYPE, T)) > func_)
        //{
        //    return ZRPC_MAKE_SHARED< Callable5<R, BOOST_PP_REPEAT(5, BOOST_PP_TYPE, T)> >(func_);
        //}

        //template<typename R, BOOST_PP_REPEAT(5, BOOST_PP_TYPENAME, T)>
        //inline ZRPC_SHARED_PTR<ICallable> makeCallable(R(*func_)(BOOST_PP_REPEAT(5, BOOST_PP_TYPE, T)))
        //{
        //    return ZRPC_MAKE_SHARED< Callable5<R, BOOST_PP_REPEAT(5, BOOST_PP_TYPE, T)> >(func_);
        //}

        //template<typename R, BOOST_PP_REPEAT(6, BOOST_PP_TYPENAME, T)>
        //inline ZRPC_SHARED_PTR<ICallable> makeCallable(ZRPC_FUNCTION < R(BOOST_PP_REPEAT(6, BOOST_PP_TYPE, T)) > func_)
        //{
        //    return ZRPC_MAKE_SHARED< Callable6<R, BOOST_PP_REPEAT(6, BOOST_PP_TYPE, T)> >(func_);
        //}

        //template<typename R, BOOST_PP_REPEAT(6, BOOST_PP_TYPENAME, T)>
        //inline ZRPC_SHARED_PTR<ICallable> makeCallable(R(*func_)(BOOST_PP_REPEAT(6, BOOST_PP_TYPE, T)))
        //{
        //    return ZRPC_MAKE_SHARED< Callable6<R, BOOST_PP_REPEAT(6, BOOST_PP_TYPE, T)> >(func_);
        //}

        //template<typename R, BOOST_PP_REPEAT(7, BOOST_PP_TYPENAME, T)>
        //inline ZRPC_SHARED_PTR<ICallable> makeCallable(ZRPC_FUNCTION < R(BOOST_PP_REPEAT(7, BOOST_PP_TYPE, T)) > func_)
        //{
        //    return ZRPC_MAKE_SHARED< Callable7<R, BOOST_PP_REPEAT(7, BOOST_PP_TYPE, T)> >(func_);
        //}

        //template<typename R, BOOST_PP_REPEAT(7, BOOST_PP_TYPENAME, T)>
        //inline ZRPC_SHARED_PTR<ICallable> makeCallable(R(*func_)(BOOST_PP_REPEAT(7, BOOST_PP_TYPE, T)))
        //{
        //    return ZRPC_MAKE_SHARED< Callable7<R, BOOST_PP_REPEAT(7, BOOST_PP_TYPE, T)> >(func_);
        //}

        //template<typename R, BOOST_PP_REPEAT(8, BOOST_PP_TYPENAME, T)>
        //inline ZRPC_SHARED_PTR<ICallable> makeCallable(ZRPC_FUNCTION < R(BOOST_PP_REPEAT(8, BOOST_PP_TYPE, T)) > func_)
        //{
        //    return ZRPC_MAKE_SHARED< Callable8<R, BOOST_PP_REPEAT(8, BOOST_PP_TYPE, T)> >(func_);
        //}

        //template<typename R, BOOST_PP_REPEAT(8, BOOST_PP_TYPENAME, T)>
        //inline ZRPC_SHARED_PTR<ICallable> makeCallable(R(*func_)(BOOST_PP_REPEAT(8, BOOST_PP_TYPE, T)))
        //{
        //    return ZRPC_MAKE_SHARED< Callable8<R, BOOST_PP_REPEAT(8, BOOST_PP_TYPE, T)> >(func_);
        //}

        //template<typename R, BOOST_PP_REPEAT(9, BOOST_PP_TYPENAME, T)>
        //inline ZRPC_SHARED_PTR<ICallable> makeCallable(ZRPC_FUNCTION < R(BOOST_PP_REPEAT(9, BOOST_PP_TYPE, T)) > func_)
        //{
        //    return ZRPC_MAKE_SHARED< Callable9<R, BOOST_PP_REPEAT(9, BOOST_PP_TYPE, T)> >(func_);
        //}

        //template<typename R, BOOST_PP_REPEAT(9, BOOST_PP_TYPENAME, T)>
        //inline ZRPC_SHARED_PTR<ICallable> makeCallable(R(*func_)(BOOST_PP_REPEAT(9, BOOST_PP_TYPE, T)))
        //{
        //    return ZRPC_MAKE_SHARED< Callable9<R, BOOST_PP_REPEAT(9, BOOST_PP_TYPE, T)> >(func_);
        //}

#ifndef ZRPC_MAKE_CALLABLE
#define ZRPC_MAKE_CALLABLE(z, n, _) \
        template<typename R, BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPENAME, T)> \
        inline ZRPC_SHARED_PTR<ICallable> makeCallable(ZRPC_FUNCTION < R(BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)) > func_) \
        { \
            return ZRPC_MAKE_SHARED< BOOST_PP_CAT(Callable, n)<R, BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)> >(func_); \
        } \
        template<typename R, BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPENAME, T)> \
        inline ZRPC_SHARED_PTR<ICallable> makeCallable(R(*func_)(BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T))) \
        { \
            return ZRPC_MAKE_SHARED< BOOST_PP_CAT(Callable, n)<R, BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)> >(func_); \
        }
        BOOST_PP_REPEAT_FROM_TO(2, 10, ZRPC_MAKE_CALLABLE, _)
#endif
#endif
    }
}
