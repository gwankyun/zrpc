#pragma once
#ifndef MSGPACK_EASY
#define MSGPACK_EASY
#include <string>
#include <sstream>
#include <utility>
#include <msgpack.hpp>

namespace msgpack
{
#ifdef __cpp_rvalue_references
#define MSGPACK_EASY_MOVE(x) std::move(x)
#else
#define MSGPACK_EASY_MOVE(x) x
#endif // __cpp_rvalue_references

    namespace easy
    {
#ifndef UINT8_MAX
        typedef unsigned char uint8_t;
#endif // !UINT8_MAX

        /**
         * @brief 序列化
         * @tparam T 需序列化的類型
         * @param v 需序列化的值
         * @return 序列化的字符串
        */
        template<typename T>
        inline std::string pack(const T& v)
        {
            using namespace std;
            stringstream buffer;
            msgpack::pack(buffer, v);
            buffer.seekg(0);
            return MSGPACK_EASY_MOVE(string(buffer.str()));
        }

        /**
         * @brief 序列化
         * @tparam T 需序列化的類型
         * @param v 需序列化的值
         * @param str 輸出的字符串
        */
        template<typename T>
        inline void pack(const T& v, std::string& str)
        {
            using namespace std;
            stringstream buffer;
            msgpack::pack(buffer, v);
            buffer.seekg(0);
            string result(buffer.str());
            str.swap(result);
        }

        /**
         * @brief 反序列化
         * @tparam T 目標類型
         * @param data 序列化數據段開始位置
         * @param len 序列化數據段長度
         * @return 目標對象
        */
        template<typename T>
        inline T unpack(const char* data, std::size_t len)
        {
            return MSGPACK_EASY_MOVE(msgpack::unpack(data, len).get().as<T>());
        }

        /**
         * @brief 反序列化
         * @tparam T 目標類型
         * @param data 序列化數據段開始位置
         * @param len 序列化數據段長度
         * @param dst 目標對象
        */
        template<typename T>
        inline void unpack(const char* data, std::size_t len, T& dst)
        {
            msgpack::object_handle oh = msgpack::unpack(data, len);
            msgpack::object deserialized = oh.get();
            deserialized.convert(dst);
        }

        /**
         * @brief 反序列化
         * @tparam T 目標類型
         * @param str 序列化字符串
         * @return 目標對象
        */
        template<typename T>
        inline T unpack(const std::string& str)
        {
            return MSGPACK_EASY_MOVE(unpack<T>(str.c_str(), str.size()));
        }

        /**
         * @brief 反序列化
         * @tparam T 目標類型
         * @param str 序列化字符串
         * @param dst 目標對象
        */
        template<typename T>
        inline void unpack(const std::string& str, T& dst)
        {
            unpack(str.c_str(), str.size(), dst);
        }

        /**
         * @brief 反序列化
         * @tparam T 目標類型
         * @param data 序列化數據段開始位置
         * @param len 序列化數據段長度
         * @return 目標對象
        */
        template<typename T>
        inline T unpack(const uint8_t* data, std::size_t len)
        {
            return MSGPACK_EASY_MOVE(unpack<T>(reinterpret_cast<const char*>(data), len));
        }

        /**
         * @brief 反序列化
         * @tparam T 目標類型
         * @param data 序列化數據段開始位置
         * @param len 序列化數據段長度
         * @param dst 目標對象
        */
        template<typename T>
        inline void unpack(const uint8_t* data, std::size_t len, T& dst)
        {
            unpack(reinterpret_cast<const char*>(data), len, dst);
        }
    }
}
#endif // !MSGPACK_EASY
