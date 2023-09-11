#include "CommandTextFilter.h"

#include <Turbo/Core/Log.h>

#include <imgui.h>

namespace Turbo::Ed
{
    namespace Utils 
    {
        static inline bool IsCharBlankA(char c)
        {
            return c == ' ' || c == '\t';
        }

        static inline char ToUpper(char c) { return (c >= 'a' && c <= 'z') ? c &= ~32 : c; }

        static const char* Stristr(const char* haystack, const char* haystack_end, const char* needle, const char* needle_end)
        {
            if (!needle_end)
                needle_end = needle + strlen(needle);

            const char un0 = (char)ToUpper(*needle);
            while ((!haystack_end && *haystack) || (haystack_end && haystack < haystack_end))
            {
                if (ToUpper(*haystack) == un0)
                {
                    const char* b = needle + 1;
                    for (const char* a = haystack + 1; b < needle_end; a++, b++)
                        if (ToUpper(*a) != ToUpper(*b))
                            break;
                    if (b == needle_end)
                        return haystack;
                }
                haystack++;
            }
            return NULL;
        }
    }

    void CommandTextFilter::TextRange::Split(char separator, std::vector<TextRange>* out) const
    {
        out->resize(0);
        const char* wb = b;
        const char* we = wb;
        while (we < e)
        {
            if (*we == separator)
            {
                out->emplace_back(wb, we);
                wb = we + 1;
            }
            we++;
        }
        if (wb != we)
            out->emplace_back(wb, we);
    }

    CommandTextFilter::CommandTextFilter()
        : m_InputBuffer{ 0 }, m_ConfirmChoice(false), CountGrep(0), m_FilteredOut(0)
    {
        AddKey("project_new");
        AddKey("project_open");
        AddKey("project_del");
        AddKey("project_love_you");
        AddKey("exit");
    }

    void CommandTextFilter::AddKey(const std::string& key)
    {
        m_Keys.push_back(key);
    }

    bool CommandTextFilter::Draw(const char* name, float width /*= 0.0f*/)
    {
        // Reset
        m_FilteredOut = 0;

        if (width != 0)
            ImGui::SetNextItemWidth(width);

        bool valueChanged = ImGui::InputText(name, m_InputBuffer, sizeof(m_InputBuffer), 0, CommandTextFilter::TextCallback, this);

        if (valueChanged)
            Build();

        for (size_t i = 0; i < m_Keys.size(); i++)
            if (PassFilter(m_Keys[i].c_str()))
                ImGui::Text("%s", m_Keys[i].c_str());
            else
                ++m_FilteredOut;

        // If nothing was selected do not confirm

        // TODO: make this properly
        static bool enterPressed = false;

        if (enterPressed)
        {
            if (ImGui::IsKeyReleased(ImGuiKey_Enter))
            {
                m_ConfirmChoice &= (m_FilteredOut != m_Filters.size());
                return m_ConfirmChoice;
            }
        }

        enterPressed = ImGui::IsKeyPressed(ImGuiKey_Enter);

        return false;
    }

    bool CommandTextFilter::PassFilter(const char* text, const char* text_end)
    {
        if (m_Keys.empty())
            return true;

        if (text == NULL)
            text = "";

        for (int i = 0; i != m_Filters.size(); i++)
        {
            const TextRange& f = m_Filters[i];
            if (f.Empty())
                continue;
            if (f.b[0] == '-')
            {
                // Subtract
                if (Utils::Stristr(text, text_end, f.b + 1, f.e) != NULL)
                    return false;
            }
            else
            {
                // Grep
                if (Utils::Stristr(text, text_end, f.b, f.e) != NULL)
                    return true;
            }
        }

        // Implicit * grep
        if (CountGrep == 0)
            return true;

        return false;
    }

    void CommandTextFilter::Build()
    {
        m_Filters.resize(0);
        size_t s = strlen(m_InputBuffer);
        TextRange input_range(m_InputBuffer, m_InputBuffer + s);
        input_range.Split(',', &m_Filters);

        CountGrep = 0;
        for (int i = 0; i != m_Filters.size(); i++)
        {
            TextRange& f = m_Filters[i];
            while (f.b < f.e && Utils::IsCharBlankA(f.b[0]))
                f.b++;
            while (f.e > f.b && Utils::IsCharBlankA(f.e[-1]))
                f.e--;
            if (f.Empty())
                continue;
            if (m_Filters[i].b[0] != '-')
                CountGrep += 1;
        }
    }

    int CommandTextFilter::TextCallback(ImGuiInputTextCallbackData* data)
    {
        if (data->EventChar == '\n')
        {
            TBO_INFO("asdasda");
        }

        TBO_INFO(data->EventChar);

        CommandTextFilter* filter = (CommandTextFilter*)data->UserData;
        filter->Build();

        /*      if (data->EventKey & ImGuiKey_Tab)
              {
                  CommandTextFilter* filter = (CommandTextFilter*)data->UserData;
                  filter->m_ConfirmChoice = true;
              }*/

        return 0;
    }

}
