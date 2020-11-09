#pragma once
#include "zrpc.h"
#include <msgpack_easy.hpp>

#if ZRPC_CXX_STD_11
#include <log.hpp>
#else
#if !defined(LOG) && !defined(LOG_IF)
#define LOG(...)
#define LOG_IF(...)
#endif
#endif // ZRPC_CXX_STD_11

namespace zrpc
{
    namespace detail
    {
        typedef unsigned __int32 uint32_t;
        typedef unsigned __int16 uint16_t;

        struct Header
        {
            char sign[4];
            uint32_t length;
            uint16_t major;
            uint16_t minor;
        };

        inline void initialize(Header& header)
        {
            memset(header.sign, '@', sizeof(header.sign));
            header.length = 0;
            header.major = 0;
            header.minor = 0;
        }

        struct Call
        {
            std::string func;
            std::string args;
            ~Call()
            {
                LOG(info, "");
            }
            MSGPACK_DEFINE(func, args);
        };

        template<typename T>
        class shared_ptr
        {
        public:
            struct Helper
            {
                Helper()
                    : data(NULL)
                    , count(1)
                {
                }

                Helper(T* data_)
                    : data(data_)
                    , count(1)
                {
                }

                ~Helper()
                {
                    if (data != NULL)
                    {
                        delete data;
                        data = NULL;
                    }
                }

                T* data;
                int count;
            };

            shared_ptr()
                : helper(NULL)
            {
                //LOG(info, "");
            }

            shared_ptr(T* data_)
                : helper(new Helper(data_))
            {
                //LOG(info, "");
            }

            shared_ptr(const shared_ptr& other)
                : helper(other.helper)
            {
                if (helper != NULL)
                {
                    helper->count++;
                }
            }

            shared_ptr& operator=(const shared_ptr& other)
            {
                if (helper != NULL)
                {
                    helper->count--;
                }
                helper = other.helper;
                helper->count++;
                return *this;
            }

            void reset()
            {
                if (helper != NULL)
                {
                    helper->count--;
                    helper = NULL;
                }
            }

            void reset(T* data_)
            {
                reset();
                helper = new Helper(data_);
            }

            ~shared_ptr()
            {
                if (helper != NULL)
                {
                    helper->count--;
                    if (helper->count == 0)
                    {
                        delete helper;
                        helper = NULL;
                    }
                }
            }

            T* get() const
            {
                if (helper == NULL)
                {
                    return NULL;
                }
                return helper->data;
            }

            T& operator*() const
            {
                //LOG(info, "helper == NULL: {}", helper == NULL);
                if (helper == NULL)
                {
                    return *((T*)0);
                }
                return *(helper->data);
            }

            T* operator->() const
            {
                return helper->data;
            }

            operator bool() const
            {
                return get() != NULL;
            }

        private:
            Helper* helper;
        };

        template<typename T>
        class vector
        {
        public:
            vector()
                : data_(NULL)
            {
            }

            vector(std::size_t size, T val)
                : data_(new T[size])
            {
                std::fill(data_, data_ + size, val);
            }

            ~vector()
            {
                if (data_ != NULL)
                {
                    delete[] data_;
                }
            }

            T* data()
            {
                return data_;
            }

        private:
            T* data_;
        };

#if ZRPC_CXX_STD_11
        template<typename Arg, typename ...Args>
        void pack(std::vector<std::string>& buffer, Arg&& arg, Args&& ...args)
        {
            buffer.push_back(msgpack::easy::pack(arg));
            pack(buffer, std::forward<Args...>(args)...);
        }

        template<typename Arg>
        void pack(std::vector<std::string>& buffer, Arg&& arg)
        {
            buffer.push_back(msgpack::easy::pack(arg));
        }

        template<typename Arg, typename ...Args>
        void unpack(std::vector<std::string>& buffer, int index, Arg& arg, Args& ...args)
        {
            msgpack::easy::unpack(buffer[index], arg);
            unpack(buffer, index + 1, std::forward<Args&...>(args)...);
        }

        template<typename Arg>
        void unpack(std::vector<std::string>& buffer, int index, Arg& arg)
        {
            msgpack::easy::unpack(buffer[index], arg);
        }
#endif // ZRPC_CXX_STD_11
    }
}
