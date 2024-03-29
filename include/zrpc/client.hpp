#pragma once
#include "zrpc.h"
#include "serialization.hpp"

#if !ZRPC_HAS_CXX_11
#  include <boost/preprocessor.hpp>
#  include <boost/function.hpp>
#  include <boost/tuple/tuple.hpp>
#  include <boost/shared_ptr.hpp>
#  include <boost/algorithm/hex.hpp>
#  include <boost/smart_ptr/enable_shared_from_this.hpp>
#  include <boost/algorithm/hex.hpp>
#  include <boost/bind/bind.hpp>
#  include "bind.hpp"
#endif

#ifndef ZRPC_USE_BOOST_OPTIONAL
#  define ZRPC_USE_BOOST_OPTIONAL 0
#endif

#include "detail.hpp"
#include "asio.hpp"

namespace zrpc
{
    namespace detail
    {
        template<typename T, typename Protocol>
#if ZRPC_HAS_CONCEPTS
    requires Protocolable<Protocol>
#endif
        typename optional<T>::type tryCall(Call call_, typename Protocol::socket& socket)
        {
            using namespace detail;
            bool enable = false;
            LOG_IF(info, enable, "call");
            std::size_t offset = 0;

            Header header;
            initialize(header);
            std::string body = pack(call_);
            header.length = (detail::uint32_t)body.size();

            while (offset < sizeof(header))
            {
                int written = socket.write_some(
                    detail::buffer((const char*)&header + offset, sizeof(header) - offset));
                offset += written;
            }
            LOG_IF(info, enable, "write header");

            offset = 0;
            while (offset < body.size())
            {
                int written = socket.write_some(
                    detail::buffer(body.c_str() + offset, body.size() - offset));
                offset += written;
            }
            LOG_IF(info, enable, "write body");

            initialize(header);
            LOG_IF(info, enable, "sizeof(header): {}", sizeof(header));
            offset = 0;
            while (offset < sizeof(header))
            {
                int written = socket.read_some(
                    detail::buffer((char*)&header + offset, sizeof(header) - offset));
                offset += written;
            }
            //zdbg("read header");

            offset = 0;

            vector<char> vec(header.length + (uint32_t)1, '\0');

            while (offset < header.length)
            {
                std::size_t written = socket.read_some(
                    detail::buffer(vec.data() + offset, header.length - offset));
                offset += written;
            }
            //zdbg("read body");

            //std::string str;
            //boost::algorithm::hex(vec.data(), vec.data() + vec.size(), std::back_inserter(str));
            //zdbg(str);

            T result;
            //bool u = tryUnpack(std::string(vec.data()), result);
            bool u = tryUnpack((const char*)vec.data(), vec.size() - 1, result);

            if (u)
            {
                return typename optional<T>::type(result);
            }
            else
            {
                return nullopt();
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
#if ZRPC_HAS_CONCEPTS
    requires Protocolable<Protocol>
#endif
        T call(Call call_, typename Protocol::socket& socket)
        {
            typename optional<T>::type result = tryCall<T, Protocol>(call_, socket);
            if (result)
            {
                return *result;
            }
            throw CallException();
        }
    }

    template<typename Protocol, typename Context>
#if ZRPC_HAS_CONCEPTS
    requires Contextable<Context> && Protocolable<Protocol>
#endif
    class Client : public ZRPC_ENABLE_SHARED_FROM_THIS< Client<Protocol, Context> >
    {
    public:
        explicit Client(
            Context& context
        ) : socket(context)
        {
        }

        ~Client()
        {
        }

        bool connect(std::string ip, uint16_t port)
        {
            try
            {
                socket.connect(Endpoint(detail::ip::make_address(ip), port));
                return true;
            }
            catch (const std::exception& e)
            {
                LOG(info, e.what());
                return false;
            }
            return false;
        }

        template<typename F>
        void asyncConnect(std::string ip, uint16_t port, F onConnect)
        {
            try
            {
                socket.async_connect(Endpoint(detail::ip::make_address(ip), port), onConnect);
            }
            catch (const std::exception& e)
            {
                LOG(info, e.what());
            }
        }

        detail::error_code lastErrorCode()
        {
            return lastErrorCode_;
        }

#if ZRPC_HAS_CXX_11
        template<typename T, typename... Args>
        typename optional<T>::type tryCall(std::string func, Args... args)
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
        template<typename R, typename T0>
        typename optional<R>::type tryCall(
            std::string func,
            T0 T0_)
        {
            using namespace detail;
            Call call;
            call.func = func;
            detail::tuple<T0> tpl(T0_);
            call.args = pack(tpl);
            return detail::tryCall<R, Protocol>(call, socket);
        }

#ifndef ZRPC_TRYCALL
#  define ZRPC_TRYCALL(z, n, _) \
        template<typename R, BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPENAME, T)> \
        typename optional<R>::type tryCall( \
            std::string func, \
            BOOST_PP_REPEAT_Z(z, n, BOOST_PP_PARAMETER, T)) \
        { \
            using namespace detail; \
            Call call; \
            call.func = func; \
            detail::tuple<BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)> tpl(BOOST_PP_REPEAT_Z(z, n, BOOST_PP_ARGUMENT, T)); \
            call.args = pack(tpl); \
            return detail::tryCall<R, Protocol>(call, socket); \
        }
        BOOST_PP_REPEAT_FROM_TO(2, 10, ZRPC_TRYCALL, _);
#endif

        template<typename R, typename T0>
        R call(
            std::string func,
            T0 T0_)
        {
            typename optional<R>::type result(tryCall<R>(func, T0_));
            if (result)
            {
                return *result;
            }
            throw detail::CallException();
        }

#ifndef ZRPC_CALL
#  define ZRPC_CALL(z, n, _) \
        template<typename R, BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPENAME, T)> \
        R call( \
            std::string func, \
            BOOST_PP_REPEAT_Z(z, n, BOOST_PP_PARAMETER, T)) \
        { \
            typename optional<R>::type result(tryCall<R>(func, BOOST_PP_REPEAT_Z(z, n, BOOST_PP_ARGUMENT, T))); \
            if (result) \
            { \
                return *result; \
            } \
            throw detail::CallException(); \
        }
        BOOST_PP_REPEAT_FROM_TO(2, 10, ZRPC_CALL, _);
#endif
#endif
    private:
        typedef typename Protocol::endpoint Endpoint;

        template<typename T, typename F>
#if ZRPC_HAS_CONCEPTS
    requires Callbackable<F, T>
#endif
        void asyncCall(const detail::Call& call, F callback)
        {
            using namespace detail;

            typedef typename shared_ptr<Header>::type HeaderPtr;
            typedef typename shared_ptr<std::string>::type BufferPtr;
            typedef typename shared_ptr<T>::type ResultPtr;

            HeaderPtr header(ZRPC_MAKE_SHARED<Header>());
            initialize(*header);
            BufferPtr buffer(ZRPC_MAKE_SHARED<std::string>(pack(call)));
            header->length = (detail::uint32_t)buffer->size();

            std::string hex;
            detail::hex(*header, std::back_inserter(hex));
            zdbg(hex, hex.size());

            ClientPtr self(this->shared_from_this());
            ResultPtr result(ZRPC_MAKE_SHARED<T>());

#if ZRPC_HAS_CXX_11
            asyncWrite(
                detail::buffer((const char*)header.get(), sizeof(*header)),
                header,
                [self, header, buffer, result, callback](error_code error, std::size_t size)
                {
                    self->onWriteHeader<T, F>(error, size, result, callback, buffer);
                });
#else
            struct Callback
            {
                Callback(F callback_, ResultPtr result_)
                    : callback(callback_)
                    , result(result_)
                {
                }
                ClientPtr self;
                ResultPtr result;
                F callback;
                BufferPtr buffer;
                void operator()(error_code error, std::size_t size)
                {
                    self->onWriteHeader<T, F>(error, size, this->result, callback, buffer);
                }
            };
            Callback cb(callback, result);
            cb.self = self;
            cb.buffer = buffer;

            asyncWrite(
                detail::buffer((const char*)header.get(), sizeof(*header)),
                header,
                cb);
#endif
        }

    public:
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
        template<typename T, typename F, typename T0>
        inline void asyncCall(std::string func, F callback, T0 T0_)
        {
            using namespace detail;
            Call call;
            call.func = func;
            call.args = pack(detail::tuple<T0>(T0_));
            asyncCall<T>(call, callback);
        }

#ifndef ZRPC_ASYNC_CALL
#  define ZRPC_ASYNC_CALL(z, n, _) \
        template<typename T, typename F, BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPENAME, T)> \
        inline void asyncCall(std::string func, F callback, BOOST_PP_REPEAT_Z(z, n, BOOST_PP_PARAMETER, T)) \
        { \
            using namespace detail; \
            Call call; \
            call.func = func; \
            call.args = pack(detail::tuple<BOOST_PP_REPEAT_Z(z, n, BOOST_PP_TYPE, T)>(BOOST_PP_REPEAT_Z(z, n, BOOST_PP_ARGUMENT, T))); \
            asyncCall<T>(call, callback); \
        }
        BOOST_PP_REPEAT_FROM_TO(2, 10, ZRPC_ASYNC_CALL, _);
#endif
#endif

    private:
        typename Protocol::socket socket;
        typedef detail::error_code error_code;
        error_code lastErrorCode_;

        typedef Client<Protocol, Context> ClientType;
        typedef typename shared_ptr< ClientType >::type ClientPtr;

        template<typename T, typename F>
#if ZRPC_HAS_CONCEPTS
    requires AsioCallbackable<F>
#endif
        void onAsyncRead(
            error_code error, std::size_t size,
            detail::mutable_buffer buffer, T data,
            std::size_t count, F f)
        {
            using namespace detail;
            if (error)
            {
                f(error, count);
                return;
            }

            std::size_t offset = count + size;
            if (offset < buffer.size())
            {
                mutable_buffer newBuffer((char*)buffer.data() + offset, buffer.size() - offset);
                ClientPtr self(this->shared_from_this());
#if ZRPC_HAS_CXX_11
                socket.async_read_some(
                    newBuffer,
                    [self, data, newBuffer, offset, f](error_code error, std::size_t size)
                    {
                        self->onAsyncRead(error, size, newBuffer, data, offset, f);
                    });
#else
                struct Callback
                {
                    void operator()(error_code error, std::size_t size, F f, mutable_buffer newBuffer, T data, std::size_t offset, ClientPtr self)
                    {
                        self->onAsyncRead(error, size, newBuffer, data, offset, f);
                    }
                };
                Callback cb;
                socket.async_read_some(
                    newBuffer,
                    boost::bind<void>(cb, _1, _2, f, newBuffer, data, offset, self));
#endif
                return;
            }
            f(error, offset);
        }

        template<typename T, typename F>
#if ZRPC_HAS_CONCEPTS
    requires AsioCallbackable<F>
#endif
        void onAsyncWrite(
            error_code error, std::size_t size,
            detail::const_buffer buffer, T data,
            std::size_t count, F f)
        {
            using namespace detail;
            if (error)
            {
                f(error, count);
                return;
            }
            std::size_t offset = count + size;
            if (offset < buffer.size())
            {
                const_buffer newBuffer(detail::buffer((const char*)buffer.data() + offset, buffer.size() - offset));
                ClientPtr self(this->shared_from_this());
#if ZRPC_HAS_CXX_11
                socket.async_write_some(
                    newBuffer,
                    [self, data, newBuffer, offset, f](error_code error, std::size_t size)
                    {
                        self->onAsyncWrite(error, size, newBuffer, data, offset, f);
                    });
#else
                struct Callback
                {
                    void operator()(error_code error, std::size_t size, F f, const_buffer newBuffer, ClientPtr self, T data, std::size_t offset)
                    {
                        self->onAsyncWrite(error, size, newBuffer, data, offset, f);
                    }
                };

                Callback cb;
                socket.async_write_some(
                    newBuffer,
                    boost::bind<void>(cb, _1, _2, f, newBuffer, self, data, offset));
#endif
                return;
            }
            f(error, offset);
        }

        template<typename T, typename F>
#if ZRPC_HAS_CONCEPTS
    requires Callbackable<F, T>
#endif
        void onReadHeader(
            error_code error, std::size_t size,
            typename shared_ptr<T>::type result, F callback,
            typename shared_ptr<detail::Header>::type header)
        {
            using namespace detail;
            if (error)
            {
                zdbg(error.message());
                return;
            }
            //zdbg("read header!");

            typedef typename shared_ptr< vector<char> >::type BufferPtr;

            BufferPtr vec(ZRPC_MAKE_SHARED< vector<char> >(header->length + (uint32_t)1, '\0'));
            ClientPtr self(this->shared_from_this());

#if ZRPC_HAS_CXX_11
            self->asyncRead(
                detail::buffer(vec->data(), vec->size() - 1),
                vec,
                [self, result, callback, vec](error_code error, std::size_t size)
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
                        callback(detail::error::invalid_argument, *result);
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

                typedef typename shared_ptr<T>::type ResultPtr;

                ClientPtr self;
                ResultPtr result;
                F callback;
                BufferPtr vec;

                void operator()(error_code error, std::size_t size)
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
                        callback(detail::error::invalid_argument, *result);
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
                detail::buffer(vec->data(), vec->size() - 1),
                vec, cb);
#endif
        }

        template<typename T, typename Fn>
#if ZRPC_HAS_CONCEPTS
    requires Callbackable<Fn, T>
#endif
        void onWriteHeader(
            error_code error, std::size_t size,
            typename shared_ptr<T>::type result, Fn callback,
            typename shared_ptr<std::string>::type buffer)
        {
            if (error)
            {
                zdbg(error.message());
                return;
            }
            zdbg("write header!");
            ClientPtr self(this->shared_from_this());

#if ZRPC_HAS_CXX_11
            asyncWrite(
                detail::buffer(buffer->c_str(), buffer->size()),
                buffer,
                [self, buffer, result, callback](error_code error, std::size_t size)
                {
                    if (error)
                    {
                        zdbg(error.message());
                        return;
                    }
                    zdbg("write body!");
                    auto header = std::make_shared<detail::Header>();
                    detail::initialize(*header);
                    self->asyncRead(
                        detail::buffer((char*)header.get(), sizeof(*header)),
                        header,
                        [self, result, callback, header](error_code error, std::size_t size)
                        {
                            self->onReadHeader<T, Fn>(error, size, result, callback, header);
                        });
                });
#else
            struct Callback
            {
                Callback(Fn f_)
                    : callback(f_)
                {
                }

                typedef typename shared_ptr<T>::type ResultPtr;
                typedef typename shared_ptr<std::string>::type BufferPtr;
                typedef typename shared_ptr<detail::Header>::type HeaderPtr;

                BufferPtr buffer;
                ClientPtr self;
                ResultPtr result;
                Fn callback;

                void operator()(error_code error, std::size_t size)
                {
                    if (error)
                    {
                        zdbg(error.message());
                        return;
                    }
                    zdbg("write body!");

                    struct Callback2
                    {
                        Callback2(Fn _callback)
                            : callback(_callback)
                        {
                        }

                        ClientPtr self;
                        HeaderPtr header;
                        ResultPtr result;
                        Fn callback;

                        void operator()(error_code error, std::size_t size)
                        {
                            self->onReadHeader<T>(error, size, result, callback, header);
                        }
                    };

                    auto header = ZRPC_MAKE_SHARED<detail::Header>();
                    detail::initialize(*header);

                    Callback2 cb2(callback);
                    cb2.header = header;
                    cb2.result = result;
                    cb2.self = self;

                    using namespace detail;

                    self->asyncRead(
                        detail::buffer((char*)header.get(), sizeof(*header)),
                        header, cb2);
                }
            };
            Callback cb(callback);
            cb.self = self;
            cb.buffer = buffer;
            cb.result = result;

            using namespace detail;

            asyncWrite(
                detail::buffer(buffer->c_str(), buffer->size()),
                buffer,
                cb
            );
#endif
        }

        template<typename T, typename F>
#if ZRPC_HAS_CONCEPTS
    requires AsioCallbackable<F>
#endif
        void asyncRead(detail::mutable_buffer buffer, T data, F f)
        {
            using namespace detail;
            ClientPtr self(this->shared_from_this());

#if ZRPC_HAS_CXX_11
            socket.async_read_some(
                buffer,
                [self, data, buffer, f](error_code error, std::size_t size)
                {
                    self->onAsyncRead(error, size, buffer, data, 0, f);
                });
#else
            struct Callback
            {
                void operator()(error_code error, std::size_t size, mutable_buffer buffer, T data, ClientPtr self, F f)
                {
                    self->onAsyncRead<T, F>(error, size, buffer, data, 0, f);
                }
            };
            Callback cb;
            socket.async_read_some(
                buffer,
                boost::bind<void>(cb, _1, _2, buffer, data, self, f));
#endif
        }

        template<typename T, typename F>
#if ZRPC_HAS_CONCEPTS
    requires AsioCallbackable<F>
#endif
        void asyncWrite(detail::const_buffer buffer, T data, F f)
        {
            using namespace detail;
            ClientPtr self(this->shared_from_this());

#if ZRPC_HAS_CXX_11
            socket.async_write_some(
                buffer,
                [self, data, buffer, f](error_code error, std::size_t size)
                {
                    self->onAsyncWrite(error, size, buffer, data, 0, f);
                });
#else
            struct Callback
            {
                void operator()(error_code error, std::size_t size, const_buffer buffer, T data, F f, ClientPtr self)
                {
                    self->onAsyncWrite(error, size, buffer, data, 0, f);
                }
            };
            Callback cb;
            socket.async_write_some(
                buffer,
                boost::bind<void>(cb, _1, _2, buffer, data, f, self));
#endif
        }
    };

    template<typename Protocol, typename Context>
#if ZRPC_HAS_CONCEPTS
    requires Contextable<Context> && Protocolable<Protocol>
#endif
    Client<Protocol, Context> makeClient(Context& context)
    {
        return Client<Protocol, Context>(context);
    }
}
