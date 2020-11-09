#pragma once
#include "spdlog/spdlog.h"
#include <spdlog/pattern_formatter.h>
#include <memory>

namespace spdlog
{
    namespace easy
    {
        namespace detail
        {
            struct Config
            {
                uint32_t filenameLength = 15;
                uint32_t funcnameLength = 20;
                uint32_t lineLength = 4;
            };

            inline Config& config()
            {
                static Config instance;
                return instance;
            }

            class formatter_flag : public spdlog::custom_flag_formatter
            {
            public:
                void format(const spdlog::details::log_msg& msg, const std::tm& tm_time, spdlog::memory_buf_t& dest) override
                {
                    std::string file(msg.source.filename);
                    std::string sub(file.substr(file.size() <= detail::config().filenameLength ? 0 : file.size() - config().filenameLength));
                    dest.append(sub.c_str(), sub.c_str() + sub.size());
                }

                std::unique_ptr<spdlog::custom_flag_formatter> clone() const override
                {
                    return spdlog::details::make_unique<formatter_flag>();
                }
            };
        }

        void initialize(uint32_t filenameLength_ = 20, uint32_t funcnameLength_ = 15, uint32_t lineLength_ = 4, const char* fmt_ = "[%Y-%m-%d %H:%M:%S.%e] [%^%8l%$] [%{0}!g] [%-{1}!!:%{2}#] %v");

        inline void initialize(uint32_t filenameLength_, uint32_t funcnameLength_, uint32_t lineLength_, const char* fmt_)
        {
            auto& config = detail::config();
            config.filenameLength = filenameLength_;
            config.funcnameLength = funcnameLength_;
            config.lineLength = lineLength_;
            auto formatter = std::make_unique<spdlog::pattern_formatter>();
            auto str = fmt::format(fmt_,
                config.filenameLength, config.funcnameLength, config.lineLength);
            formatter->add_flag<detail::formatter_flag>('g').set_pattern(str);
            spdlog::set_formatter(std::move(formatter));
        }
    }
}

#if !defined(LOG) && !defined(LOG_IF)
#define LOG(level_, ...) SPDLOG_LOGGER_CALL(spdlog::default_logger_raw(), spdlog::level::level_, __VA_ARGS__)
#define LOG_IF(level_, condition_, ...) if ((condition_)) { LOG(level_, __VA_ARGS__); }
#endif
