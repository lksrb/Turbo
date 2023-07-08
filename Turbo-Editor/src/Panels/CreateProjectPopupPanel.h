#pragma once

#include <Turbo/Editor/EditorPanel.h>

#include <filesystem>
#include <functional>

namespace Turbo::Ed
{
    using CreateProjectCallback = std::function<void(const std::filesystem::path&)>;

    // Modal window
    class CreateProjectPopupPanel : public EditorPanel
    {
    public:
        CreateProjectPopupPanel(const CreateProjectCallback& callback);
        ~CreateProjectPopupPanel();

        void Open();
        void OnDrawUI() override;
    private:
        CreateProjectCallback m_Callback;
        bool m_Open = false;
    };
}
