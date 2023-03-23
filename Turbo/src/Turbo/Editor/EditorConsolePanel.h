#pragma once

#include "EditorPanel.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <mutex>

#include <iostream>

namespace Turbo
{
    struct ConsoleMessage
    {
        std::string Message;
        Log::Level MessageLevel;
        u32 Count = 1; // TODO: For repeated messages
    };

    class EditorConsolePanel : public EditorPanel
    {
    public:
        EditorConsolePanel();
        ~EditorConsolePanel();
        void OnDrawUI() override;
        void OnEvent(Event& e) override;

        static void PushMessage(const ConsoleMessage& message);
    private:
        void PushMessageInternal(const ConsoleMessage& message);

        std::array<ConsoleMessage, 500> m_MessageBuffer;
        u32 m_MessageBufferCount = 0;
    };

    class EditorConsoleSink : public spdlog::sinks::base_sink<std::mutex>
    {
    public:
        explicit EditorConsoleSink()
        {
            m_MessageQueue.resize(m_MessageQueueMax);
        }
        virtual ~EditorConsoleSink() = default;

        EditorConsoleSink(const EditorConsoleSink& other) = delete;
        EditorConsoleSink& operator=(const EditorConsoleSink& other) = delete;

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override
        {
            spdlog::memory_buf_t formatted;
            spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg, formatted);

            m_MessageQueue[m_MessageCount++] = ConsoleMessage{fmt::to_string(formatted), GetMessageLevel(msg.level)};

            if (m_MessageCount == m_MessageQueueMax)
                flush_();
        }

        void flush_() override
        {
            for (const auto& msg : m_MessageQueue)
                EditorConsolePanel::PushMessage(msg);

            m_MessageCount = 0;
        }
    private:
        static Log::Level  GetMessageLevel(spdlog::level::level_enum level)
        {
            switch (level)
            {
                case spdlog::level::trace:
                case spdlog::level::debug:
                case spdlog::level::info:
                    return Log::Level::Info;
                case spdlog::level::warn:
                    return Log::Level::Warn;
                case spdlog::level::err:
                case spdlog::level::critical:
                    return Log::Level::Error;
            }

            TBO_ENGINE_ASSERT("Invalid Message Level!");
            return Log::Level::Trace;
        }
    private:
        std::vector<ConsoleMessage> m_MessageQueue;
        u32 m_MessageCount = 0;
        const u32 m_MessageQueueMax = 1;
    };
}
