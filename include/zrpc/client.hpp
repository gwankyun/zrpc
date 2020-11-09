#pragma once
#include "zrpc.h"
#include <asio.hpp>
#include "detail.hpp"
#include <boost/preprocessor.hpp>
#include <boost/function.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

//msgpack::easy::pack(msgpack::type::make_tuple<>())

//#define CLIENT_CALL(x) \
//    template<typename T, EXTEND(T, TYPENAME, x, COMMA)> \
//    T call(std::string func, EXTEND(T, ARG, x, COMMA)) \
//    { \
//        using namespace detail; \
//        bool enable = false; \
//        std::size_t offset = 0; \
//        Call call; \
//        call.func = func; \
//        call.args = pack(EXTEND(T, CALL, x, COMMA)); \
//        return detail::call<T, Protocol>(call, socket); \
//    }

//#define

#if !ZRPC_HAS_CXX_11

#ifndef ZRPC_TYPENAME
#  define ZRPC_TYPENAME(z, n, x) , typename x##n
#endif // !ZRPC_TYPENAME

#ifndef ZRPC_TYPE
#  define ZRPC_TYPE(z, n, x) BOOST_PP_COMMA_IF(n) x##n
#endif // !ZRPC_TYPE

#ifndef BOOST_PP_REPEAT_Z
#  define BOOST_PP_REPEAT_Z(z) BOOST_PP_REPEAT_##z
#endif // !BOOST_PP_REPEAT_Z

#define ZRPC_CALL(z, n, _) \
    template<typename T BOOST_PP_REPEAT_Z(n)(n, ZRPC_TYPENAME, T)> \
    T call(std::string func BOOST_PP_COMMA_IF(n) BOOST_PP_REPEAT_Z(n)(T, ARG, x, COMMA)) \
    { \
        using namespace detail; \
        bool enable = false; \
        std::size_t offset = 0; \
        Call call; \
        call.func = func; \
        call.args = msgpack::easy::pack(msgpack::type::make_tuple<>  pack(EXTEND(T, CALL, x, COMMA)); \
        return detail::call<T, Protocol>(call, socket); \
    }

#endif // !ZRPC_HAS_CXX_11


namespace zrpc
{
    namespace detail
    {
        //template<typename T, typename Protocol>
        //T call(Call call_, typename Protocol::socket& socket)
        //{
        //    using namespace detail;
        //    bool enable = false;
        //    LOG_IF(info, enable, "call");
        //    std::size_t offset = 0;

        //    Header header;
        //    initialize(header);
        //    std::string body = msgpack::easy::pack(call_);
        //    header.length = body.size();

        //    while (offset < sizeof(header))
        //    {
        //        int written = socket.write_some(
        //            asio::buffer((const char*)&header + offset, sizeof(header) - offset));
        //        offset += written;
        //    }
        //    LOG_IF(info, enable, "write header");

        //    offset = 0;
        //    while (offset < body.size())
        //    {
        //        int written = socket.write_some(
        //            asio::buffer(body.c_str() + offset, body.size() - offset));
        //        offset += written;
        //    }
        //    LOG_IF(info, enable, "write body");

        //    initialize(header);
        //    LOG_IF(info, enable, "sizeof(header): {}", sizeof(header));
        //    offset = 0;
        //    while (offset < sizeof(header))
        //    {
        //        int written = socket.read_some(
        //            asio::buffer((char*)&header + offset, sizeof(header) - offset));
        //        offset += written;
        //    }
        //    LOG_IF(info, enable, "read header");

        //    offset = 0;

        //    detail::vector<char> vec(header.length + (uint32_t)1, '\0');

        //    while (offset < header.length)
        //    {
        //        std::size_t written = socket.read_some(
        //            asio::buffer(vec.data() + offset, header.length - offset));
        //        offset += written;
        //    }
        //    LOG_IF(info, enable, "read body");
        //    T result;
        //    zrpc::unpack(std::string(vec.data()), result);

        //    return result;
        //}

        template<typename T, typename Protocol>
        T call(Call call_, typename Protocol::socket& socket)
        {
            using namespace detail;
            bool enable = false;
            LOG_IF(info, enable, "call");
            std::size_t offset = 0;

            Header header;
            initialize(header);
            std::string body = msgpack::easy::pack(call_);
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
            LOG_IF(info, enable, "read header");

            offset = 0;

            detail::vector<char> vec(header.length + (uint32_t)1, '\0');

            while (offset < header.length)
            {
                std::size_t written = socket.read_some(
                    asio::buffer(vec.data() + offset, header.length - offset));
                offset += written;
            }
            LOG_IF(info, enable, "read body");
            T result;
            //zrpc::unpack(std::string(vec.data()), result);
            msgpack::easy::unpack(std::string(vec.data()), result);

            return result;
        }
    }

    template<typename Protocol>
    class Client
    {
    public:
        //explicit Client(
        //    ZRPC_SHARED_PTR<asio::ip::tcp::socket> socket_
        //) : socket(socket_)
        //{

        //}

        explicit Client(
            io_context& context
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

#if ZRPC_CXX_STD_11
        //template<typename T, typename ...Args>
        //T call(std::string func, Args ...args)
        //{
        //    using namespace detail;
        //    bool enable = false;
        //    LOG_IF(info, enable, "call");
        //    std::size_t offset = 0;
        //    Call call;
        //    call.func = func;
        //    call.args = pack(std::forward<Args>(args)...);

        //    return detail::call<T, Protocol>(call, socket);
        //}

        template<typename T, typename... Args>
        T call(std::string func, Args... args)
        {
            using namespace detail;
            bool enable = false;
            LOG_IF(info, enable, "call");
            std::size_t offset = 0;
            Call call;
            call.func = func;
            //call.args = pack(std::forward<Args>(args)...);
            call.args = msgpack::easy::pack(std::make_tuple<Args...>(std::forward<Args>(args)...));

            return detail::call<T, Protocol>(call, socket);
        }
#else
        DEFINE(CLIENT_CALL)
#endif // ZRPC_CXX_STD_11

    private:
        typename Protocol::socket socket;
    };
}
