#pragma once
#include "zrpc.h"
#include <string>
#include <utility> // std::forward
//#include <msgpack_easy.hpp>

#include <msgpack.hpp>

//namespace msgpack
//{
//    namespace easy
//    {
//        template<typename T>
//        inline void pack(const T& value, std::string& buffer)
//        {
//            std::stringstream sbuffer;
//            msgpack::pack(sbuffer, value);
//            sbuffer.seekg(0);
//            sbuffer.str().swap(buffer);
//        }
//
//        template<typename T>
//        inline std::string pack(const T& value)
//        {
//            std::string buffer;
//            pack(value, buffer);
//            return buffer;
//        }
//
//        template<typename T>
//        inline void unpack(const std::string& buffer, T& value)
//        {
//            msgpack::object_handle oh = msgpack::unpack(buffer.data(), buffer.size());
//            msgpack::object obj = oh.get();
//            std::cout << obj << std::endl;
//            obj.convert(value);
//        }
//
//        template<typename T>
//        inline T unpack(const std::string& buffer)
//        {
//            T value;
//            unpack(buffer, value);
//            return value;
//        }
//    }
//}

#define ZRPC_ASSERT(...)

#define UNPACK_VECTOR(x, n) \
    msgpack::easy::unpack(vec[n - 1], x##n)

#define PUSH_BACK(x, n) \
    buffer.push_back(msgpack::easy::pack(x##n))

#define PACK(x) \
    template<EXTEND(T, TYPENAME, x, COMMA)> \
    std::string pack(EXTEND(T, ARG, x, COMMA)) \
    { \
        std::vector<std::string> buffer; \
        EXTEND(_T, PUSH_BACK, x, SEMICOLON); \
        return msgpack::easy::pack(buffer); \
    }

#define UNPACK(x) \
    template<EXTEND(T, TYPENAME, x, COMMA)> \
    void unpack(std::string buffer, EXTEND(T, REF_ARG, x, COMMA)) \
    { \
        std::vector<std::string> vec; \
        msgpack::easy::unpack(buffer, vec); \
        ZRPC_ASSERT(vec.size() == x); \
        EXTEND(_T, UNPACK_VECTOR, x, SEMICOLON); \
    }

namespace zrpc
{
#if ZRPC_CXX_STD_11
    template<typename Arg, typename ...Args>
    std::string pack(Arg&& arg, Args&& ...args)
    {
        std::vector<std::string> buffer;
        detail::pack(buffer, arg, std::forward<Args>(args)...);
        return msgpack::easy::pack(buffer);
    }

    template<typename Arg, typename ...Args>
    void unpack(std::string buffer, Arg& arg, Args& ...args)
    {
        std::vector<std::string> vec;
        msgpack::easy::unpack(buffer, vec);
        detail::unpack(vec, 0, arg, std::forward<Args&>(args)...);
    }
#else
    DEFINE(PACK)

    DEFINE(UNPACK)

#endif // ZRPC_CXX_STD_11
}
