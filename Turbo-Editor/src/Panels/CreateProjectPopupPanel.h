#pragma once

#include <Turbo/Editor/EditorPanel.h>

#include <filesystem>
#include <functional>

namespace Turbo::Ed {

    // Modal window
    class CreateProjectPopupPanel : public EditorPanel
    {
    public:
        using Callback = std::function<void(const std::filesystem::path&)>;

        CreateProjectPopupPanel(const Callback& callback);
        ~CreateProjectPopupPanel();

        void Open();
        void OnDrawUI() override;
    private:
        Callback m_Callback;
        bool m_Open = false;
    };
}
