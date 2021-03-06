#pragma once
#include <map> // std::map
#include <functional> // std::function
#include "zrpc.h"
#include <asio.hpp>
#include <asio/coroutine.hpp>
#include <boost/algorithm/hex.hpp>

#if ZRPC_HAS_CXX_11
#  include <tuple>
#else
#  include <boost/preprocessor.hpp>
#  include <boost/function.hpp>
#  include <boost/tuple/tuple.hpp>
#  include <boost/shared_ptr.hpp>
#  include <boost/make_shared.hpp>
#endif // ZRPC_HAS_CXX_11

#ifndef ZRPC_USING
#  if ZRPC_HAS_CXX_11
#    define ZRPC_USING(n, o) using n = o
#  else
#    define ZRPC_USING(n, o) typedef o n
#  endif // ZRPC_HAS_CXX_11
#endif // !ZRPC_USING

#if defined(__cpp_lib_apply)
#  define ZRPC_APPLY(fn, ...) std::apply(fn, ##__VA_ARGS__)
#else
#  include <apply.hpp>
#  define ZRPC_APPLY(fn, ...) lite::apply(fn, ##__VA_ARGS__)
#endif // defined(__cpp_lib_apply)

namespace zrpc
{
    inline asio::ASIO_CONST_BUFFER writeBuffer(const void* data, std::size_t size_in_bytes, std::size_t offset)
    {
        return asio::buffer(
            (const char*)data + offset, size_in_bytes - offset);
    }

    inline asio::ASIO_MUTABLE_BUFFER readBuffer(void* data, std::size_t size_in_bytes, std::size_t offset)
    {
        return asio::buffer(
            (char*)data + offset, size_in_bytes - offset);
    }

    template<typename S, typename F>
    inline void asyncWrite(S& socket, const void* data, std::size_t size_in_bytes, std::size_t offset, F fn)
    {
        socket.async_write_some(
            writeBuffer(data, size_in_bytes, offset),
            fn);
    }

    template<typename S, typename F>
    inline void asyncRead(S& socket, void* data, std::size_t size_in_bytes, std::size_t offset, F fn)
    {
        socket.async_read_some(
            readBuffer(data, size_in_bytes, offset),
            fn);
    }

    template<typename S, typename E, typename F>
    inline void asyncConnect(S& socket, E endpoint, F fn)
    {
        socket.async_connect(endpoint, fn);
    }

    template<typename S, typename T, typename F>
    inline void asyncWait(S& socket, T& timer, F fn)
    {
#if ZRPC_HAS_CXX_11
        timer.async_wait([fn, &socket](asio::error_code error)
            {
                if (error)
                {
                    zdbg(error.message());
                    return;
                }
                zdbg("timeout!");
                socket.close();
                return;
});
#else
        struct Callback
        {
            Callback(S& socket_, F& fn_)
                : socket(socket_)
                , fn(fn_)
            {
            }
            F& fn;
            S& socket;
            void operator()(asio::error_code error)
            {
                if (error)
                {
                    zdbg(error.message());
                    return;
                }
                zdbg("timeout!");
                socket.close();
                return;
            }
        };
        Callback callback(socket, fn);
        timer.async_wait(callback);
#endif // ZRPC_HAS_CXX_11
}

    template<typename Protocol>
    class Server
    {
    public:
        explicit Server(
            typename Protocol::acceptor& acceptor_
        )
            : acceptor(&acceptor_)
            , child(false), enable(false), offset(0)
            , timeout(0)
        {
        }

        explicit Server(
            asio::io_context& context,
            Protocol protocol, uint16_t port
        )
            : acceptor(new Protocol::acceptor(context, Protocol::endpoint(protocol, port)))
            , child(false), enable(false), offset(0)
        {
        }


        ~Server()
        {
        }

        void setTimeout(uint32_t milliseconds)
        {
            timeout = milliseconds;
        }

        ZRPC_USING(FnType, ZRPC_SHARED_PTR<detail::ICallable>);

        std::string invoke(const detail::Call& inArg)
        {
            std::map<std::string, FnType>::iterator iter = process->find(inArg.func);
            if (iter != process->end())
            {
                return iter->second->call(inArg.args);
            }
            else
            {
                return detail::pack(std::string(""));
            }
        }

        template<typename Fn>
        void bind(std::string func, Fn fn)
        {
            if (!process)
            {
                process.reset(new std::map<std::string, FnType>());
            }
            (*process)[func] = detail::makeCallable(fn);
        }

        void operator()(asio::error_code error = asio::error_code(), std::size_t bytes_transferred = 0);

    private:
        asio::coroutine coro;
        std::size_t offset;
        bool child;
        bool enable;
        bool result;
        uint32_t timeout;

        ZRPC_SHARED_PTR< typename Protocol::acceptor > acceptor;
        ZRPC_SHARED_PTR< typename Protocol::socket > socket;
        ZRPC_SHARED_PTR< detail::Call > call;
        ZRPC_SHARED_PTR< std::string > send;
        ZRPC_SHARED_PTR< detail::Header > header;
        ZRPC_SHARED_PTR< detail::vector<char> > recv;
        ZRPC_SHARED_PTR< std::string > hex;
        ZRPC_SHARED_PTR< std::map<std::string, FnType> > process;
        ZRPC_SHARED_PTR< asio::steady_timer > timer;
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

                header.reset(new Header());
                offset = 0;
                do
                {
                    ASIO_CORO_YIELD
                    {
                        if (timeout != 0)
                        {
                            timer.reset(new asio::steady_timer(socket->get_executor(), asio::chrono::milliseconds(timeout)));
                            asyncWait(*socket, *timer, *this);
                        }
                        asyncRead(*socket, header.get(), sizeof(*header), offset, *this);
                    }
                    if (timeout != 0)
                    {
                        timer->cancel();
                    }
                    offset += bytes_transferred;
                } while (offset < sizeof(*header));
                LOG_IF(info, enable, "read header: {}", header->length);

                hex.reset(new std::string());
                detail::hex(*header, std::back_inserter(*hex));
                zdbg(*hex, hex->size());

                recv.reset(new detail::vector<char>(header->length + 1, '\0'));
                offset = 0;
                do
                {
                    ASIO_CORO_YIELD
                    {
                        if (timeout != 0)
                        {
                            timer.reset(new asio::steady_timer(socket->get_executor(), asio::chrono::milliseconds(timeout)));
                            asyncWait(*socket, *timer, *this);
                        }
                        asyncRead(*socket, recv->data(), header->length, offset, *this);
                    }
                    if (timeout != 0)
                    {
                        timer->cancel();
                    }
                    offset += bytes_transferred;
                } while (offset < header->length);
                LOG_IF(info, enable, "read body");

                hex.reset(new std::string());
                boost::algorithm::hex(recv->data(), recv->data() + recv->size(), std::back_inserter(*hex));
                zdbg(*hex);

                call.reset(new Call());
                result = tryUnpack(recv->data(), recv->size(), *call);
                zdbg(result);
                LOG_IF(info, enable, "func: {}", call->func);

                send.reset(new std::string());
                *send = invoke(*call);

                initialize(*header);
                header->length = send->size();

                offset = 0;
                do
                {
                    ASIO_CORO_YIELD
                    {
                        if (timeout != 0)
                        {
                            timer.reset(new asio::steady_timer(socket->get_executor(), asio::chrono::milliseconds(timeout)));
                            asyncWait(*socket, *timer, *this);
                        }
                        asyncWrite(*socket, header.get(), sizeof(*header), offset, *this);
                    }
                    if (timeout != 0)
                    {
                        timer->cancel();
                    }
                    offset += bytes_transferred;
                } while (offset < sizeof(*header));
                LOG_IF(info, enable, "write header: {}", header->length);

                offset = 0;
                do
                {
                    ASIO_CORO_YIELD
                    {
                        if (timeout != 0)
                        {
                            timer.reset(new asio::steady_timer(socket->get_executor(), asio::chrono::milliseconds(timeout)));
                            asyncWait(*socket, *timer, *this);
                        }
                        asyncWrite(*socket, send->c_str(), send->size(), offset, *this);
                    }
                    if (timeout != 0)
                    {
                        timer->cancel();
                    }
                    offset += bytes_transferred;
                } while (offset < send->size());
                LOG_IF(info, enable, "write body");
            }
        }
        else
        {
            zdbg(error.message());
        }
    }
}
