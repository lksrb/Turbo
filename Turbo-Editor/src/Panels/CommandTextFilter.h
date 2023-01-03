#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

#include <vector>
#include <string>

typedef int ImGuiInputTextFlags;
struct ImGuiInputTextCallbackData;

namespace Turbo::Ed {

    // Helper class
    class CommandTextFilter {
    public:
        CommandTextFilter();
        void AddKey(const std::string& key);
        bool Draw(const char* name, float width /*= 0.0f*/);
        const char* GetText() const { return m_InputBuffer; }
    private:
        struct TextRange {
            const char* b;
            const char* e;

            TextRange() { b = e = NULL; }
            TextRange(const char* _b, const char* _e) { b = _b; e = _e; }
            bool Empty() const { return b == e; }
            void Split(char separator, std::vector<TextRange>* out) const;
        };

        bool PassFilter(const char* text, const char* text_end = nullptr);
        void Build();
        static int TextCallback(ImGuiInputTextCallbackData* data);
    private:
        std::vector<TextRange> m_Filters;
        std::vector<std::string> m_Keys;

        u32 m_FilteredOut;
        bool m_ConfirmChoice;
        char m_InputBuffer[256];
        int CountGrep;
    };

}
