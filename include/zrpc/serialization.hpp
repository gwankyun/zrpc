#pragma once
#include "zrpc.h"
#include <string>
#include <msgpack.hpp>

namespace zrpc
{
    template<typename T>
    inline void pack(const T& value, std::string& buffer)
    {
        std::stringstream sbuffer;
        msgpack::pack(sbuffer, value);
        sbuffer.seekg(0);
        sbuffer.str().swap(buffer);
    }

    template<typename T>
    inline std::string pack(const T& value)
    {
        std::string buffer;
        pack(value, buffer);
        return buffer;
    }

    template<typename T>
    inline void unpack(const std::string& buffer, T& value)
    {
        msgpack::object_handle oh = msgpack::unpack(buffer.c_str(), buffer.size());
        msgpack::object obj = oh.get();
        obj.convert(value);
    }

    template<typename T>
    inline void unpack(const char* data, std::size_t len, T& value)
    {
        msgpack::object_handle oh = msgpack::unpack(data, len);
        msgpack::object obj = oh.get();
        obj.convert(value);
    }

    template<typename T>
    inline T unpack(const std::string& buffer)
    {
        T value;
        unpack(buffer, value);
        return value;
    }
}
