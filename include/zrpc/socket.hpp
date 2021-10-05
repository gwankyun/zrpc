#pragma once
#include "asio.hpp"

namespace zrpc
{
    namespace detail
    {
#if ZRPC_HAS_CONCEPTS
    template<typename T>
    concept Fnable = requires (T fn, error_code error, std::size_t size)
    {
        fn(error, size);
    };
#endif

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
#if ZRPC_HAS_CONCEPTS
    requires Fnable<F>
#endif
        inline void asyncWrite(S& socket, const void* data, std::size_t size_in_bytes, std::size_t offset, F fn)
        {
            socket.async_write_some(
                writeBuffer(data, size_in_bytes, offset),
                fn);
        }

        template<typename S, typename F>
#if ZRPC_HAS_CONCEPTS
    requires Fnable<F>
#endif
        inline void asyncRead(S& socket, void* data, std::size_t size_in_bytes, std::size_t offset, F fn)
        {
            socket.async_read_some(
                readBuffer(data, size_in_bytes, offset),
                fn);
        }

        template<typename S, typename E, typename F>
#if ZRPC_HAS_CONCEPTS
    requires Fnable<F>
#endif
        inline void asyncConnect(S& socket, E endpoint, F fn)
        {
            socket.async_connect(endpoint, fn);
        }

        template<typename S, typename T, typename F>
#if ZRPC_HAS_CONCEPTS
    requires Fnable<F>
#endif
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
}
