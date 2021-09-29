#pragma once
#include "zrpc.h"
#include <map> // std::map
#include <functional> // std::function
#include "serialization.hpp"
#include "detail.hpp"
#include "coroutine.hpp"
#include <boost/algorithm/hex.hpp>

#if ZRPC_HAS_CXX_11
#  include <tuple>
#else
#  include <boost/preprocessor.hpp>
#  include <boost/function.hpp>
#  include <boost/tuple/tuple.hpp>
#  include <boost/shared_ptr.hpp>
#  include <boost/make_shared.hpp>
#endif

#include "asio.hpp"

namespace zrpc
{
    namespace detail
    {
        inline const_buffer writeBuffer(const void* data, std::size_t size_in_bytes, std::size_t offset)
        {
            return buffer(
                (const char*)data + offset,
                size_in_bytes - offset);
        }

        inline mutable_buffer readBuffer(void* data, std::size_t size_in_bytes, std::size_t offset)
        {
            return buffer(
                (char*)data + offset,
                size_in_bytes - offset);
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
            timer.async_wait([fn, &socket](error_code error)
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
                void operator()(error_code error)
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
#endif
        }
    }

    template<typename Context, typename Protocol>
    class Server
    {
    public:
        //explicit Server(
        //    Acceptor& acceptor_
        //)
        //    : acceptor(&acceptor_)
        //    , child(false), enable(false), offset(0)
        //    , timeout(0)
        //{
        //}

        explicit Server(
            Context& context,
            Protocol protocol,
            uint16_t port
        )
            : acceptor(new Acceptor(context, Protocol::endpoint(protocol, port)))
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

        typedef typename shared_ptr<detail::ICallable>::type FnType;

        template<typename Fn>
        void bind(std::string func, Fn fn)
        {
            if (!process)
            {
                process.reset(new FnMap());
            }
            (*process)[func] = detail::makeCallable(fn);
        }

        void operator()(detail::error_code error = detail::error_code(), std::size_t bytes_transferred = 0);

    private:
        detail::coroutine coro;
        std::size_t offset;
        bool child;
        bool enable;
        bool result;
        uint32_t timeout;

        typedef typename Protocol::socket Socket;
        typedef typename Protocol::acceptor Acceptor;
        typedef std::map< std::string, FnType > FnMap;

        typename shared_ptr< Acceptor >::type acceptor;
        typename shared_ptr< Socket >::type socket;
        typename shared_ptr< detail::Call >::type call;
        typename shared_ptr< std::string >::type send;
        typename shared_ptr< detail::Header >::type header;
        typename shared_ptr< detail::vector<char> >::type recv;
        typename shared_ptr< std::string >::type hex;
        typename shared_ptr< FnMap >::type process;
        typename shared_ptr< detail::steady_timer >::type _timer;

        void wait(typename shared_ptr< detail::steady_timer >::type timer)
        {
            if (timeout != 0)
            {
                timer.reset(new detail::steady_timer(socket->get_executor(), detail::chrono::milliseconds(timeout)));
                detail::asyncWait(*socket, *timer, *this);
            }
        }

        void cancelWait(typename shared_ptr< detail::steady_timer >::type timer)
        {
            if (timeout != 0)
            {
                if (timer)
                {
                    timer->cancel();
                }
            }
        }

        std::string invoke(const detail::Call& inArg)
        {
            FnMap::iterator iter = process->find(inArg.func);
            if (iter != process->end())
            {
                return iter->second->call(inArg.args);
            }
            else
            {
                return detail::pack(std::string(""));
            }
        }

        template<typename F>
        void asyncInvoke(const detail::Call& inArg, F fn)
        {
            *send = invoke(inArg);
            fn(detail::error_code(), 0);
        }
    };

    template<typename Protocol , typename Context>
    void Server<Protocol, Context>::operator()(detail::error_code error, std::size_t bytes_transferred)
    {
        enable = true;
        if (!error)
        {
            ZRPC_CORO_REENTER(coro) while (true)
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
                    socket.reset(new Socket(acceptor->get_executor()));
                    ZRPC_CORO_YIELD acceptor->async_accept(*socket, *this);
                    ZRPC_CORO_FORK Server(*this)();
                } while (coro.is_parent());

                child = true;

                // async_read_some
                header.reset(new Header());
                initialize(*header);

                header.reset(new Header());
                offset = 0;
                do
                {
                    ZRPC_CORO_YIELD
                    {
                        wait(_timer);
                        asyncRead(*socket, header.get(), sizeof(*header), offset, *this);
                    };
                    cancelWait(_timer);
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
                    ZRPC_CORO_YIELD
                    {
                        wait(_timer);
                        asyncRead(*socket, recv->data(), header->length, offset, *this);
                    };
                    cancelWait(_timer);
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

                ZRPC_CORO_YIELD
                {
                    asyncInvoke(*call, *this);
                };

                initialize(*header);
                header->length = (detail::uint32_t)send->size();

                offset = 0;
                do
                {
                    ZRPC_CORO_YIELD
                    {
                        wait(_timer);
                        asyncWrite(*socket, header.get(), sizeof(*header), offset, *this);
                    };
                    cancelWait(_timer);
                    offset += bytes_transferred;
                } while (offset < sizeof(*header));
                LOG_IF(info, enable, "write header: {}", header->length);

                offset = 0;
                do
                {
                    ZRPC_CORO_YIELD
                    {
                        wait(_timer);
                        asyncWrite(*socket, send->c_str(), send->size(), offset, *this);
                    };
                    cancelWait(_timer);
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

    template<typename Context, typename Protocol>
    Server<Context, Protocol> makeServer(
        Context& context,
        Protocol protocol,
        uint16_t port
    )
    {
        return Server<Context, Protocol>(context, protocol, port);
    }
}
