#pragma once

#include "Turbo/Core/Common.h"
#include "Turbo/Core/String.h"

#include "CommandTextFilter.h"

#include <functional>

namespace Turbo::Ed {

    using OnInputSendCallback = std::function<void(const String&)>;

    enum AccessPanelMode : u32 {
        AccessPanelMode_Command = 0,
        AccessPanelMode_OpenFile,
        AccessPanelMode_NewFile,
    };

    class AccessPanel {
    public:
        AccessPanel();
        ~AccessPanel();

        void OnUIRender();
        void Open(bool open = true); 

        bool IsOpened() const;
        void SetOnInputSendCallback(OnInputSendCallback callback);

        void SetMode(AccessPanelMode mode) { m_Mode = mode; }
    private:
        void ImGuiCenterWindow();
    private:
        AccessPanelMode m_Mode;
        bool m_Open;
        CommandTextFilter m_TextFilter;// TODO: Text Filteting
        OnInputSendCallback m_Callback;
        String m_Input;
        char m_TextBuffer[64];
    };

}
