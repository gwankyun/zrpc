#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <cstddef>
#include <utility>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <cstdint> // std::uint32_t
#include <cstring> // memset
#include <map> // std::map
#include <functional>
#include <msgpack_easy.hpp>
#include <asio.hpp>
#include <asio/coroutine.hpp>
#include <asio/yield.hpp>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>

#ifndef ASYNC_WRITE
#define ASYNC_WRITE(socket_, data_, len_, per_, offset_, bytes_transferred_) \
    offset_ = 0; \
    do \
    { \
        yield socket_->async_write_some( \
            asio::buffer( \
                (const char*)data_ + offset_, \
                std::min(len_ - offset_, (std::size_t)per_)), *this); \
        offset_ += bytes_transferred_; \
    } while (offset_ < len_);
#endif // !ASYNC_WRITE

#ifndef ASYNC_READ
#define ASYNC_READ(socket_, data_, len_, per_, offset_, bytes_transferred_) \
    offset_ = 0; \
    do \
    { \
        yield socket_->async_read_some( \
            asio::buffer( \
                (char*)data_ + offset_, \
                std::min(len_ - offset_, (std::size_t)per_)), *this); \
        offset_ += bytes_transferred_; \
    } while (offset_ < len_);
#endif // !ASYNC_READ

#if !defined(LOG) && !defined(LOG_IF)
#define LOG(level_, ...) SPDLOG_LOGGER_CALL(spdlog::default_logger_raw(), spdlog::level::level_, __VA_ARGS__)
#define LOG_IF(level_, condition_, ...) if ((condition_)) { LOG(level_, __VA_ARGS__); }
#endif

namespace zrpc
{
    struct Header
    {
        char sign[4];
        std::uint32_t length;
    };

    void initialize(Header& header)
    {
        memset(header.sign, '@', sizeof(header.sign));
        header.length = 0;
    }

    template<typename ...Args>
    std::string pack(Args ...args)
    {
        auto args_ = std::make_tuple<Args...>(std::forward<Args>(args)...);
        return msgpack::easy::pack(args_);
    }

    template<typename ...Args>
    void unpack(std::string buffer, Args& ...args)
    {
        std::tuple<Args&...> arg_(std::tuple<Args&...>(std::forward<Args&>(args)...));
        msgpack::easy::unpack(buffer, arg_);
    }

    struct Call
    {
        std::string func;
        std::string args;
        MSGPACK_DEFINE(func, args);
    };

    class Server
    {
    public:
        explicit Server(
            asio::ip::tcp::acceptor& acceptor_,
            std::shared_ptr<asio::ip::tcp::socket> socket_
        )
            : acceptor(acceptor_), socket(socket_)
        {
        }

        ~Server()
        {
        }

        std::string invoke(const Call& inArg)
        {
            auto iter = process->find(inArg.func);
            if (iter != process->end())
            {
                return iter->second(inArg.args);
            }
            else
            {
                return msgpack::easy::pack(std::string(""));
            }
        }

        template<typename Fn>
        void bind(std::string func, Fn fn)
        {
            if (!process)
            {
                process.reset(new std::map<std::string, std::function<std::string(std::string)>>());
            }
            (*process)[func] = [=](std::string str) -> std::string {
                return fn(str);
            };
        }

        void operator()(asio::error_code error = asio::error_code(), std::size_t bytes_transferred = 0);

    private:
        asio::ip::tcp::acceptor& acceptor;
        std::shared_ptr<asio::ip::tcp::socket> socket;
        asio::coroutine coro;

        std::shared_ptr<Call> call;
        std::size_t offset = 0;
        std::shared_ptr<std::string> send;
        std::shared_ptr<Header> header;
        std::shared_ptr<std::vector<char>> recv;
        std::shared_ptr<std::map<std::string, std::function<std::string(std::string)>>> process;
        bool child = false;
        bool enable = false;
    };

    void Server::operator()(asio::error_code error, std::size_t bytes_transferred)
    {
        if (!error)
        {
            reenter(coro) while (true)
            {
                enable = false;
                // async_accept
                do
                {
                    if (child)
                    {
                        LOG_IF(info, enable, "child");
                        break;
                    }
                    LOG_IF(info, enable, "async_accept");
                    socket.reset(new asio::ip::tcp::socket(acceptor.get_executor()));
                    yield acceptor.async_accept(*socket, *this);
                    fork Server(*this)();
                } while (coro.is_parent());

                child = true;

                // async_read_some
                header.reset(new Header());
                initialize(*header);

                ASYNC_READ(socket, header.get(), sizeof(*header), sizeof(*header), offset, bytes_transferred);
                LOG_IF(info, enable, "read header: {}", header->length);

                recv.reset(new std::vector<char>(header->length + 1, '\0'));
                ASYNC_READ(socket, recv->data(), header->length, 3, offset, bytes_transferred);
                LOG_IF(info, enable, "read body");

                call.reset(new Call());
                msgpack::easy::unpack(recv->data(), *call);
                LOG_IF(info, enable, "func: {}", call->func);

                send.reset(new std::string());
                *send = invoke(*call);

                initialize(*header);
                header->length = send->size();

                ASYNC_WRITE(socket, header.get(), sizeof(*header), sizeof(*header), offset, bytes_transferred);
                LOG_IF(info, enable, "write header: {}", header->length);

                ASYNC_WRITE(socket, send->c_str(), send->size(), send->size(), offset, bytes_transferred);
                LOG_IF(info, enable, "write body");
            }
        }
    }

    class Client
    {
    public:
        explicit Client(
            std::shared_ptr<asio::ip::tcp::socket> socket_
        ) : socket(socket_)
        {

        }

        ~Client()
        {
        }

        void connect(std::string ip, uint16_t port)
        {
            try
            {
                socket->connect(asio::ip::tcp::endpoint(asio::ip::make_address(ip), port));
            }
            catch (const std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }
        }

        template<typename T, typename ...Args>
        T call(std::string func, Args ...args)
        {
            bool enable = false;
            LOG_IF(info, enable, "call");
            int offset = 0;
            Call call;
            call.func = func;
            call.args = pack(std::forward<Args>(args)...);

            Header header;
            initialize(header);
            auto body = msgpack::easy::pack(call);
            header.length = body.size();

            while (offset < sizeof(header))
            {
                int written = socket->write_some(
                    asio::buffer((const char*)&header + offset, sizeof(header) - offset));
                offset += written;
            }
            LOG_IF(info, enable, "write header");

            offset = 0;
            while (offset < body.size())
            {
                int written = socket->write_some(
                    asio::buffer(body.c_str() + offset, body.size() - offset));
                offset += written;
            }
            LOG_IF(info, enable, "write body");

            initialize(header);
            LOG_IF(info, enable, "sizeof(header): {}", sizeof(header));
            offset = 0;
            while (offset < sizeof(header))
            {
                int written = socket->read_some(
                    asio::buffer((char*)&header + offset, sizeof(header) - offset));
                offset += written;
            }
            LOG_IF(info, enable, "read header");

            offset = 0;
            std::vector<char> vec(header.length + 1, '\0');
            while (offset < header.length)
            {
                auto written = socket->read_some(
                    asio::buffer(vec.data() + offset, header.length - offset));
                offset += written;
            }
            LOG_IF(info, enable, "read body");
            return msgpack::easy::unpack<T>(vec.data());
        }

    private:
        std::shared_ptr<asio::ip::tcp::socket> socket;
    };

}
