#pragma once
#include <map> // std::map
#include <functional> // std::function
#include <tuple>
//#include <tuple>
#include "zrpc.h"
#include <asio.hpp>
#include <asio/coroutine.hpp>
//#include <asio/yield.hpp>
#include <boost/preprocessor.hpp>
#include <boost/function.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#ifndef ASYNC_WRITE
#define ASYNC_WRITE(socket_, data_, len_, per_, offset_, bytes_transferred_) \
    offset_ = 0; \
    do \
    { \
        ASIO_CORO_YIELD socket_->async_write_some( \
            asio::buffer( \
                (const char*)data_ + offset_, \
                std::min(len_ - offset_, (std::size_t)per_)), *this); \
        offset_ += bytes_transferred_; \
    } while (offset_ < len_);
#endif // !ASYNC_WRITE

#ifndef ASYNC_WRITE_ALL
#define ASYNC_WRITE_ALL(socket_, data_, len_, offset_, bytes_transferred_) \
    ASYNC_WRITE(socket_, data_, len_, len_, offset_, bytes_transferred_)
#endif // !ASYNC_WRITE_ALL

#ifndef ASYNC_READ
#define ASYNC_READ(socket_, data_, len_, per_, offset_, bytes_transferred_) \
    offset_ = 0; \
    do \
    { \
        ASIO_CORO_YIELD socket_->async_read_some( \
            asio::buffer( \
                (char*)data_ + offset_, \
                std::min(len_ - offset_, (std::size_t)per_)), *this); \
        offset_ += bytes_transferred_; \
    } while (offset_ < len_);
#endif // !ASYNC_READ

#ifndef ASYNC_READ_ALL
#define ASYNC_READ_ALL(socket_, data_, len_, offset_, bytes_transferred_) \
    ASYNC_READ(socket_, data_, len_, len_, offset_, bytes_transferred_)
#endif // !ASYNC_READ

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
        std::tuple<Args...> args;
        msgpack::easy::unpack(param, args);
        R result = std::apply(func, args);
        return msgpack::easy::pack(result);
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

#ifndef ZRPC_TYPENAME
#  define ZRPC_TYPENAME(z, n, x) , typename x##n
#endif // !ZRPC_TYPENAME

#ifndef ZRPC_TYPE
#  define ZRPC_TYPE(z, n, x) BOOST_PP_COMMA_IF(n) x##n
#endif // !ZRPC_TYPE

#ifndef BOOST_PP_REPEAT_Z
#  define BOOST_PP_REPEAT_Z(z) BOOST_PP_REPEAT_##z
#endif // !BOOST_PP_REPEAT_Z

#define CALLABLE(z, n, _) \
    template<typename R BOOST_PP_REPEAT_Z(z)(n, ZRPC_TYPENAME, T)> \
    struct BOOST_PP_CAT(Callable, n) : public ICallable \
    { \
        BOOST_PP_CAT(Callable, n)(boost::function<R(BOOST_PP_REPEAT_Z(z)(n, ZRPC_TYPE, T))> func_) \
            : func(func_) \
        { \
        } \
        ~BOOST_PP_CAT(Callable, n)() \
        { \
        } \
        std::string call(std::string param) \
        { \
            msgpack::type::tuple<BOOST_PP_REPEAT_Z(z)(n, ZRPC_TYPE, T)> args; \
            msgpack::easy::unpack(param, args); \
            R result; \
            return msgpack::easy::pack(result); \
        } \
        std::function<R(BOOST_PP_REPEAT_Z(z)(n, ZRPC_TYPE, T))> func; \
    }; \
    template<typename R BOOST_PP_REPEAT_Z(z)(n, ZRPC_TYPENAME, T)> \
    boost::shared_ptr<ICallable> makeCallable(boost::function<R(BOOST_PP_REPEAT_Z(z)(n, ZRPC_TYPE, T))> func_) \
    { \
        return booot::make_shared<BOOST_PP_CAT(Callable, n)<R BOOST_PP_COMMA_IF(n) BOOST_PP_REPEAT_Z(z)(n, ZRPC_TYPE, T)> >(func_); \
    } \
    template<typename R BOOST_PP_REPEAT_Z(z)(n, ZRPC_TYPENAME, T)> \
    boost::shared_ptr<ICallable> makeCallable(R(*func_)(BOOST_PP_REPEAT_Z(z)(n, ZRPC_TYPE, T))) \
    { \
        return boost::make_shared<BOOST_PP_CAT(Callable, n)<R BOOST_PP_COMMA_IF(n) BOOST_PP_REPEAT_Z(z)(n, ZRPC_TYPE, T)> >(func_); \
    }

BOOST_PP_REPEAT_FROM_TO(1, 10, CALLABLE, _)
#endif // ZRPC_HAS_CXX_11

namespace zrpc
{
    template<typename Protocol>
    class Server
    {
    public:
        explicit Server(
            typename Protocol::acceptor& acceptor_
        )
            : acceptor(&acceptor_)
            , child(false), enable(false), offset(0)
        {
        }

        explicit Server(
            io_context& context,
            Protocol protocol, uint16_t port
        )
            : acceptor(new Protocol::acceptor(context, Protocol::endpoint(protocol, port)))
            , child(false), enable(false), offset(0)
        {
        }


        ~Server()
        {
        }

#if ZRPC_CXX_STD_11
        using FnType = std::shared_ptr<ICallable>;
#else
        //typedef std::string(*FnType)(std::string);
        typedef boost::shared_ptr<ICallable> FnType;
#endif // ZRPC_CXX_STD_11

        std::string invoke(const detail::Call& inArg)
        {
            std::map<std::string, FnType>::iterator iter = process->find(inArg.func);
            if (iter != process->end())
            {
                return iter->second->call(inArg.args);
            }
            else
            {
                return msgpack::easy::pack(std::string(""));
            }
        }

#if ZRPC_CXX_STD_11
        template<typename Fn>
        void bind(std::string func, Fn fn)
        {
            if (!process)
            {
                process.reset(new std::map<std::string, FnType>());
            }
            (*process)[func] = makeCallable(fn);
        }
#else
        //void bind(std::string func, FnType fn)
        //{
        //    if (!process)
        //    {
        //        process.reset(new std::map<std::string, FnType>());
        //    }
        //    (*process)[func] = fn;
        //}

        template<typename Fn>
        void bind(std::string func, Fn fn)
        {
            if (!process)
            {
                process.reset(new std::map<std::string, FnType>());
            }
            (*process)[func] = makeCallable(fn);
        }
#endif // ZRPC_CXX_STD_11

        void operator()(asio::error_code error = asio::error_code(), std::size_t bytes_transferred = 0);

    private:
        asio::coroutine coro;
        std::size_t offset;
        bool child;
        bool enable;

        detail::shared_ptr<typename Protocol::acceptor> acceptor;
        detail::shared_ptr<typename Protocol::socket> socket;
        detail::shared_ptr<detail::Call> call;
        detail::shared_ptr<std::string> send;
        detail::shared_ptr<detail::Header> header;
        detail::shared_ptr<detail::vector<char>> recv;
        detail::shared_ptr<std::map<std::string, FnType>> process;
    };

    template<typename Protocol>
    void Server<Protocol>::operator()(asio::error_code error, std::size_t bytes_transferred)
    {
        enable = true;
        if (!error)
        {
            ASIO_CORO_REENTER(coro) while (true)
            {
                using namespace detail;
                // async_accept
                do
                {
                    if (child)
                    {
                        LOG_IF(info, enable, "child");
                        break;
                    }
                    LOG_IF(info, enable, "async_accept");
                    socket.reset(new Protocol::socket(acceptor->get_executor()));
                    ASIO_CORO_YIELD acceptor->async_accept(*socket, *this);
                    ASIO_CORO_FORK Server(*this)();
                } while (coro.is_parent());

                child = true;

                // async_read_some
                header.reset(new Header());
                initialize(*header);

                ASYNC_READ_ALL(socket, header.get(), sizeof(*header), offset, bytes_transferred);
                LOG_IF(info, enable, "read header: {}", header->length);

                recv.reset(new detail::vector<char>(header->length + 1, '\0'));
                ASYNC_READ_ALL(socket, recv->data(), header->length, offset, bytes_transferred);
                LOG_IF(info, enable, "read body");

                call.reset(new Call());
                msgpack::easy::unpack(recv->data(), *call);
                LOG_IF(info, enable, "func: {}", call->func);

                send.reset(new std::string());
                *send = invoke(*call);

                initialize(*header);
                header->length = send->size();

                ASYNC_WRITE_ALL(socket, header.get(), sizeof(*header), offset, bytes_transferred);
                LOG_IF(info, enable, "write header: {}", header->length);

                ASYNC_WRITE_ALL(socket, send->c_str(), send->size(), offset, bytes_transferred);
                LOG_IF(info, enable, "write body");
            }
        }
        else
        {
            LOG_IF(info, enable, error.message());
        }
    }
}
