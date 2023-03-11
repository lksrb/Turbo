#pragma once

#include <Turbo/Core/Common.h>

#include "CommandTextFilter.h"
#include "Panel.h"

#include <Turbo/Event/KeyEvent.h>

namespace Turbo::Ed 
{
    class QuickAccessPanel : public Panel
    {
    public:
        QuickAccessPanel();
        ~QuickAccessPanel();

        void Open(bool open = true); 
        bool IsOpened() const { return m_Open; }

        void OnDrawUI() override;
        void OnEvent(Event& e) override;

        bool OnKeyPressed(KeyPressedEvent& e);
    private:
        bool m_Open = false;
        CommandTextFilter m_TextFilter;// TODO: Text Filteting
        std::string m_Input;
        char m_TextBuffer[64] = {};
    };
}
