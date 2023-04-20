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
        enum Category : u32
        {
            Info = TBO_BIT(0),
            Warn = TBO_BIT(1),
            Error = TBO_BIT(2),
        };

        ConsoleMessage() = default;

        ConsoleMessage(const std::string& text, Category category)
            : Text(text), MessageCategory(category)
        {
            // Current time screws up hashing so we need to do the substring
            ID = std::hash<std::string>{}(text.substr(text.find_first_of(']') + 1));
            ID += MessageCategory;
        }
    private:
        std::string Text;
        Category MessageCategory = Category::Info;
        u64 ID = 0;
        u32 Count = 1;

        friend class EditorConsolePanel;
    };

    class EditorConsolePanel : public EditorPanel
    {
    public:
        EditorConsolePanel();
        ~EditorConsolePanel();
        void OnDrawUI() override;
        void OnEvent(Event& e) override;

        static void PushMessage(const ConsoleMessage& message);
        static void Clear();
    private:
        void DrawMessages();
        void PushMessageInternal(const ConsoleMessage& message);

        u32 m_MessageFilter = ConsoleMessage::Category::Info | ConsoleMessage::Category::Warn | ConsoleMessage::Category::Error;
        bool m_AutoScroll = true; // TODO: Option in project/editor
        std::array<ConsoleMessage, 256> m_MessageBuffer;
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

            m_MessageQueue[m_MessageCount++] = ConsoleMessage(fmt::to_string(formatted), GetMessageCategory(msg.level));

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
        static ConsoleMessage::Category GetMessageCategory(spdlog::level::level_enum level)
        {
            switch (level)
            {
                case spdlog::level::trace:
                case spdlog::level::debug:
                case spdlog::level::info:
                    return ConsoleMessage::Category::Info;
                case spdlog::level::warn:
                    return ConsoleMessage::Category::Warn;
                case spdlog::level::err:
                case spdlog::level::critical:
                    return ConsoleMessage::Category::Error;
            }

            TBO_ENGINE_ASSERT("Invalid Message Level!");
            return ConsoleMessage::Category::Info;
        }
    private:
        std::vector<ConsoleMessage> m_MessageQueue;
        u32 m_MessageCount = 0;
        const u32 m_MessageQueueMax = 1;
    };
}
