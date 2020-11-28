#pragma once
#include "zrpc.h"
#include <asio.hpp>
#include "detail.hpp"
#include <boost/preprocessor.hpp>
#include <boost/function.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/algorithm/hex.hpp>

#ifndef ZRPC_USE_BOOST_OPTIONAL
#  define ZRPC_USE_BOOST_OPTIONAL 0
#endif // !ZRPC_USE_BOOST_OPTIONAL

#if !defined(ZPRC_OPTIONAL) && !defined(ZRPC_NULLOPT)
#  if ZRPC_HAS_CXX_17 && !ZRPC_USE_BOOST_OPTIONAL
#    include <optional>
#    define ZPRC_OPTIONAL std::optional
#    define ZRPC_NULLOPT std::nullopt
#  else
#    include <boost/optional.hpp>
#    define ZPRC_OPTIONAL boost::optional
#    define ZRPC_NULLOPT boost::none
#  endif // ZRPC_HAS_CXX_17 && !ZRPC_USE_BOOST_OPTIONAL
#endif // !defined(ZPRC_OPTIONAL) && !defined(ZRPC_NULLOPT)

namespace zrpc
{
    namespace detail
    {
        template<typename T, typename Protocol>
        ZPRC_OPTIONAL<T> tryCall(Call call_, typename Protocol::socket& socket)
        {
            using namespace detail;
            bool enable = false;
            LOG_IF(info, enable, "call");
            std::size_t offset = 0;

            Header header;
            initialize(header);
            std::string body = pack(call_);
            header.length = body.size();

            while (offset < sizeof(header))
            {
                int written = socket.write_some(
                    asio::buffer((const char*)&header + offset, sizeof(header) - offset));
                offset += written;
            }
            LOG_IF(info, enable, "write header");

            offset = 0;
            while (offset < body.size())
            {
                int written = socket.write_some(
                    asio::buffer(body.c_str() + offset, body.size() - offset));
                offset += written;
            }
            LOG_IF(info, enable, "write body");

            initialize(header);
            LOG_IF(info, enable, "sizeof(header): {}", sizeof(header));
            offset = 0;
            while (offset < sizeof(header))
            {
                int written = socket.read_some(
                    asio::buffer((char*)&header + offset, sizeof(header) - offset));
                offset += written;
            }
            zdbg("read header");

            offset = 0;

            detail::vector<char> vec(header.length + (uint32_t)1, '\0');

            while (offset < header.length)
            {
                std::size_t written = socket.read_some(
                    asio::buffer(vec.data() + offset, header.length - offset));
                offset += written;
            }
            zdbg("read body");

            //std::string str;
            //boost::algorithm::hex(vec.data(), vec.data() + vec.size(), std::back_inserter(str));
            //zdbg(str);

            T result;
            //bool u = tryUnpack(std::string(vec.data()), result);
            bool u = tryUnpack((const char*)vec.data(), vec.size() - 1, result);

            if (u)
            {
                return ZPRC_OPTIONAL<T>(result);
            }
            else
            {
                return ZRPC_NULLOPT;
            }
        }

        struct CallException : public std::exception
        {
            char const* what() const
            {
                return "invalid argument to unpack";
            }
        };

        template<typename T, typename Protocol>
        T call(Call call_, typename Protocol::socket& socket)
        {

            ZPRC_OPTIONAL<T> result = tryCall<T, Protocol>(call_, socket);
            if (result)
            {
                return *result;
            }
            throw CallException();
        }
    }

    template<typename Protocol>
    class Client : public ZRPC_ENABLE_SHARED_FROM_THIS< Client<Protocol> >
    {
    public:
        explicit Client(
            asio::io_context& context
        ) : socket(context)
        {
        }

        ~Client()
        {
        }

        void connect(std::string ip, uint16_t port)
        {
            try
            {
                socket.connect(Protocol::endpoint(asio::ip::make_address(ip), port));
            }
            catch (const std::exception& e)
            {
                LOG(info, e.what());
            }
        }

        template<typename F>
        void asyncConnect(std::string ip, uint16_t port, F onConnect)
        {
            try
            {
                socket.async_connect(Protocol::endpoint(asio::ip::make_address(ip), port), onConnect);
            }
            catch (const std::exception& e)
            {
                LOG(info, e.what());
            }
        }

        asio::error_code lastErrorCode()
        {
            return lastErrorCode_;
        }

#if ZRPC_HAS_CXX_11
        template<typename T, typename... Args>
        ZPRC_OPTIONAL<T> tryCall(std::string func, Args... args)
        {
            using namespace detail;
            bool enable = false;
            LOG_IF(info, enable, "call");
            std::size_t offset = 0;
            Call call;
            call.func = func;
            call.args = pack(std::make_tuple<Args...>(std::forward<Args>(args)...));

            return detail::tryCall<T, Protocol>(call, socket);
        }

        template<typename T, typename... Args>
        T call(std::string func, Args... args)
        {
            auto result = tryCall<T>(func, std::forward<Args>(args)...);
            if (result)
            {
                return *result;
            }
            throw detail::CallException();
        }
#else
#  define ZRPC_TRY_CALL(z, n, _) \
    template< \
        typename R BOOST_PP_COMMA_IF(n) \
        BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPENAME, T)> \
    ZPRC_OPTIONAL<R> tryCall( \
        std::string func BOOST_PP_COMMA_IF(n) \
        BOOST_PP_REPEAT_Z(z, n, BOOST_PP_PARAMETER, T)) \
    { \
        using namespace detail; \
        Call call; \
        call.func = func; \
        msgpack::type::tuple<BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)> tpl(BOOST_PP_REPEAT_Z(z, n, BOOST_PP_ARGUMENT, T)); \
        call.args = pack(tpl); \
        return detail::tryCall<R, Protocol>(call, socket); \
    }
    BOOST_PP_REPEAT(10, ZRPC_TRY_CALL, _)
#  define ZRPC_CALL(z, n, _) \
    template< \
        typename R BOOST_PP_COMMA_IF(n) \
        BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPENAME, T)> \
    R call( \
        std::string func BOOST_PP_COMMA_IF(n) \
        BOOST_PP_REPEAT_Z(z, n, BOOST_PP_PARAMETER, T)) \
    { \
        ZPRC_OPTIONAL<R> result(tryCall<R>(func, BOOST_PP_REPEAT_Z(z, n, BOOST_PP_ARGUMENT, T))); \
        if (result) \
        { \
             return *result; \
        } \
        throw detail::CallException(); \
    }
    BOOST_PP_REPEAT(10, ZRPC_CALL, _)
#endif // ZRPC_HAS_CXX_11

    private:
        template<typename T, typename F>
        void onAsyncWrite(
            asio::error_code error, std::size_t size,
            asio::ASIO_CONST_BUFFER buffer, T data,
            std::size_t count, F f)
        {
            if (error)
            {
                f(error, count);
                return;
            }
            std::size_t offset = count + size;
            if (offset < buffer.size())
            {
                asio::ASIO_CONST_BUFFER newBuffer(asio::buffer(buffer.begin() + offset, buffer.size() - offset));
                ZRPC_SHARED_PTR< Client<Protocol> > self(shared_from_this());
#if ZRPC_HAS_CXX_11
                socket.async_write_some(
                    newBuffer,
                    [self, data, newBuffer, offset, f](asio::error_code error, std::size_t size)
                    {
                        self->onAsyncWrite(error, size, newBuffer, data, offset, f);
                    });
#else
                struct Callback
                {
                    ZRPC_SHARED_PTR< Client<Protocol> > self;
                    asio::ASIO_CONST_BUFFER newBuffer;
                    T data;
                    std::size_t offset;
                    F f;
                    Callback(F f_, asio::ASIO_CONST_BUFFER newBuffer_)
                        : f(f_)
                        , newBuffer(newBuffer_)
                    {
                    }
                    void operator()(asio::error_code error, std::size_t size)
                    {
                        self->onAsyncWrite(error, size, newBuffer, data, offset, f);
                    }
                };
                Callback cb(f, newBuffer);
                cb.self = self;
                cb.data = data;
                cb.offset = offset;
                socket.async_write_some(newBuffer, cb);
#endif
                return;
            }
            f(error, offset);
        }

        template<typename T, typename F>
        void onAsyncRead(
            asio::error_code error, std::size_t size,
            asio::ASIO_MUTABLE_BUFFER buffer, T data,
            std::size_t count, F f)
        {
            if (error)
            {
                f(error, count);
                return;
            }
            std::size_t offset = count + size;
            if (offset < buffer.size())
            {
                asio::ASIO_MUTABLE_BUFFER newBuffer((char*)buffer.data() + offset, buffer.size() - offset);
                ZRPC_SHARED_PTR< Client<Protocol> > self(shared_from_this());
#if ZRPC_HAS_CXX_11
                socket.async_read_some(
                    newBuffer,
                    [self, data, newBuffer, offset, f](asio::error_code error, std::size_t size)
                    {
                        self->onAsyncRead(error, size, newBuffer, data, offset, f);
                    });
#else
                struct Callback
                {
                    ZRPC_SHARED_PTR< Client<Protocol> > self;
                    asio::ASIO_MUTABLE_BUFFER newBuffer;
                    T data;
                    std::size_t offset;
                    F f;
                    Callback(F f_, asio::ASIO_MUTABLE_BUFFER newBuffer_)
                        : f(f_)
                        , newBuffer(newBuffer_)
                    {
                    }
                    void operator()(asio::error_code error, std::size_t size)
                    {
                        self->onAsyncRead(error, size, newBuffer, data, offset, f);
                    }
                };
                Callback cb(f, newBuffer);
                cb.data = data;
                cb.offset = offset;
                socket.async_read_some(newBuffer, cb);
#endif
                return;
            }
            f(error, offset);
        }

        template<typename T, typename F>
        void onReadHeader(
            asio::error_code error, std::size_t size,
            ZRPC_SHARED_PTR<T> result, F callback,
            ZRPC_SHARED_PTR<detail::Header> header)
        {
            if (error)
            {
                zdbg(error.message());
                return;
            }
            zdbg("read header!");
            ZRPC_SHARED_PTR< detail::vector<char> > vec(ZRPC_MAKE_SHARED<detail::vector<char>>(header->length + (uint32_t)1, '\0'));
            ZRPC_SHARED_PTR< Client<Protocol> > self(shared_from_this());
#if ZRPC_HAS_CXX_11
            self->asyncRead(
                asio::buffer(vec->data(), vec->size() - 1),
                vec,
                [self, result, callback, vec](asio::error_code error, std::size_t size)
                {
                    if (error)
                    {
                        zdbg(error.message());
                        return;
                    }
                    zdbg("read buffer!");
                    bool u = detail::tryUnpack(vec->data(), vec->size() - 1, *result);
                    if (!u)
                    {
                        callback(asio::error::invalid_argument, *result);
                        return;
                    }
                    callback(error, *result);
                });
#else
            struct Callback
            {
                Callback(F callback_)
                    : callback(callback_)
                {
                }
                ZRPC_SHARED_PTR< Client<Protocol> > self;
                ZRPC_SHARED_PTR<T> result;
                F callback;
                ZRPC_SHARED_PTR< detail::vector<char> > vec;
                void operator()(asio::error_code error, std::size_t size)
                {
                    if (error)
                    {
                        zdbg(error.message());
                        return;
                    }
                    zdbg("read buffer!");
                    bool u = detail::tryUnpack(vec->data(), vec->size() - 1, *result);
                    if (!u)
                    {
                        callback(asio::error::invalid_argument, *result);
                        return;
                    }
                    callback(error, *result);
                }
            };
            Callback cb(callback);
            cb.self = self;
            cb.result = result;
            cb.vec = vec;
            self->asyncRead(
                asio::buffer(vec->data(), vec->size() - 1),
                vec, cb);
#endif
        }

        template<typename T, typename F>
        void onWriteHeader(
            asio::error_code error, std::size_t size,
            ZRPC_SHARED_PTR<T> result, F callback,
            ZRPC_SHARED_PTR<std::string> buffer)
        {
            if (error)
            {
                zdbg(error.message());
                return;
            }
            zdbg("write header!");
            ZRPC_SHARED_PTR< Client<Protocol> > self(shared_from_this());

#if ZRPC_HAS_CXX_11
            asyncWrite(
                asio::buffer(buffer->c_str(), buffer->size()),
                buffer,
                [self, buffer, result, callback](asio::error_code error, std::size_t size)
                {
                    if (error)
                    {
                        zdbg(error.message());
                        return;
                    }
                    zdbg("write body!");
                    auto header = ZRPC_MAKE_SHARED<detail::Header>();
                    detail::initialize(*header);
                    self->asyncRead(
                        asio::buffer((char*)header.get(), sizeof(*header)),
                        header,
                        [self, result, callback, header](asio::error_code error, std::size_t size)
                        {
                            self->onReadHeader(error, size, result, callback, header);
                        });
                });
#else
            struct Callback
            {
                Callback(F f_)
                    : callback(f_)
                {
                }
                ZRPC_SHARED_PTR< Client<Protocol> > self;
                ZRPC_SHARED_PTR<T> result;
                F callback;
                ZRPC_SHARED_PTR<std::string> buffer;
                void operator()(asio::error_code error, std::size_t size)
                {
                    if (error)
                    {
                        zdbg(error.message());
                        return;
                    }
                    zdbg("write body!");

                    struct Callback2
                    {
                        Callback2(F f_)
                            : callback(f_)
                        {
                        }
                        ZRPC_SHARED_PTR< Client<Protocol> > self;
                        ZRPC_SHARED_PTR<detail::Header> header;
                        ZRPC_SHARED_PTR<T> result;
                        F callback;
                        void operator()(asio::error_code error, std::size_t size)
                        {
                            self->onReadHeader(error, size, result, callback, header);
                        }
                    };

                    auto header = ZRPC_MAKE_SHARED<detail::Header>();
                    detail::initialize(*header);

                    Callback2 cb2(callback);
                    cb2.header = header;
                    cb2.result = result;
                    cb2.self = self;

                    self->asyncRead(
                        asio::buffer((char*)header.get(), sizeof(*header)),
                        header, cb2);
                }
            };
            Callback cb(callback);
            cb.self = self;
            cb.buffer = buffer;
            cb.result = result;

            asyncWrite(
                asio::buffer(buffer->c_str(), buffer->size()),
                buffer, cb);
#endif
        }

    public:
        template<typename T, typename F>
        void asyncWrite(asio::ASIO_CONST_BUFFER buffer, T data, F f)
        {
            ZRPC_SHARED_PTR< Client<Protocol> > self(shared_from_this());

#if ZRPC_HAS_CXX_11
            socket.async_write_some(
                buffer,
                [self, data, buffer, f](asio::error_code error, std::size_t size)
                {
                    self->onAsyncWrite(error, size, buffer, data, 0, f);
                });
#else
            struct Callback
            {
                Callback(asio::ASIO_CONST_BUFFER buffer_, F f_)
                    : buffer(buffer_)
                    , f(f_)
                {
                }
                ZRPC_SHARED_PTR< Client<Protocol> > self;
                asio::ASIO_CONST_BUFFER buffer;
                T data;
                F f;
                void operator()(asio::error_code error, std::size_t size)
                {
                    self->onAsyncWrite(error, size, buffer, data, 0, f);
                }
            };
            Callback cb(buffer, f);
            cb.data = data;
            cb.self = self;
            socket.async_write_some(buffer, cb);
#endif
        }

        template<typename T, typename F>
        void asyncRead(asio::ASIO_MUTABLE_BUFFER buffer, T data, F f)
        {
            ZRPC_SHARED_PTR< Client<Protocol> > self(shared_from_this());

#if ZRPC_HAS_CXX_11
            socket.async_read_some(
                buffer,
                [self, data, buffer, f](asio::error_code error, std::size_t size)
                {
                    self->onAsyncRead(error, size, buffer, data, 0, f);
                });
#else
            struct Callback
            {
                Callback(asio::ASIO_MUTABLE_BUFFER buffer_, F f_)
                    : buffer(buffer_)
                    , f(f_)
                {
                }
                ZRPC_SHARED_PTR< Client<Protocol> > self;
                asio::ASIO_MUTABLE_BUFFER buffer;
                T data;
                F f;
                void operator()(asio::error_code error, std::size_t size)
                {
                    self->onAsyncRead(error, size, buffer, data, 0, f);
                }
            };
            Callback cb(buffer, f);
            cb.data = data;
            cb.self = self;
            socket.async_read_some(buffer, cb);
#endif // false && ZRPC_HAS_CXX_11
        }

        template<typename T, typename F>
        void asyncCall(const detail::Call& call, F callback)
        {
            using namespace detail;
            ZRPC_SHARED_PTR<Header> header(ZRPC_MAKE_SHARED<Header>());
            initialize(*header);
            ZRPC_SHARED_PTR<std::string> buffer(ZRPC_MAKE_SHARED<std::string>(pack(call)));
            header->length = buffer->size();

            std::string hex;
            detail::hex(*header, std::back_inserter(hex));
            zdbg(hex, hex.size());

            ZRPC_SHARED_PTR< Client<typename Protocol> > self(shared_from_this());
            ZRPC_SHARED_PTR<T> result(ZRPC_MAKE_SHARED<T>());

#if ZRPC_HAS_CXX_11
            asyncWrite(
                asio::buffer((const char*)header.get(), sizeof(*header)),
                header,
                [self, header, buffer, result, callback](asio::error_code error, std::size_t size)
                {
                    self->onWriteHeader(error, size, result, callback, buffer);
                });
#else
            struct Callback
            {
                Callback(F callback_)
                    : callback(callback_)
                {
                }
                ZRPC_SHARED_PTR< Client<Protocol> > self;
                ZRPC_SHARED_PTR<T> result;
                F callback;
                ZRPC_SHARED_PTR<std::string> buffer;
                void operator()(asio::error_code error, std::size_t size)
                {
                    self->onWriteHeader(error, size, result, callback, buffer);
                }
            };
            Callback cb(callback);
            cb.self = self;
            cb.result = result;
            cb.buffer = buffer;

            asyncWrite(
                asio::buffer((const char*)header.get(), sizeof(*header)),
                header,
                cb);
#endif // ZRPC_HAS_CXX_11
        }

#if ZRPC_HAS_CXX_11
        template<typename T, typename F, typename... Args>
        void asyncCall(std::string func, F callback, Args... args)
        {
            using namespace detail;
            Call call;
            call.func = func;
            call.args = pack(std::make_tuple<Args...>(std::forward<Args>(args)...));
            asyncCall<T>(call, callback);
        }
#else
#  define ZRPC_ASYNC_CALL(z, n, _) \
    template< \
        typename T, typename F BOOST_PP_COMMA_IF(n) \
        BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPENAME, T)> \
    void asyncCall( \
        std::string func, \
        F callback BOOST_PP_COMMA_IF(n) \
        BOOST_PP_REPEAT_Z(z, n, BOOST_PP_PARAMETER, T)) \
    { \
        using namespace detail; \
        Call call; \
        call.func = func; \
        call.args = pack(msgpack::type::tuple<BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)>(BOOST_PP_REPEAT_Z(z, n, BOOST_PP_ARGUMENT, T))); \
        asyncCall<T>(call, callback); \
    }
    BOOST_PP_REPEAT(10, ZRPC_ASYNC_CALL, _)
#endif

    private:
        typename Protocol::socket socket;
        asio::error_code lastErrorCode_;
    };
}
