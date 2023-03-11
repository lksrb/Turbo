#pragma once

#include <filesystem>
#include <functional>

#include "Panel.h"

namespace Turbo::Ed
{
    using CreateProjectCallback = std::function<void(const std::filesystem::path&)>;

    // Modal window
    class CreateProjectPopupPanel : public Panel
    {
    public:
        CreateProjectPopupPanel();
        ~CreateProjectPopupPanel();

        void SetCallback(const CreateProjectCallback& callback);

        void Open() { m_Open = true; }
        void OnDrawUI() override;
        void OnEvent(Event& e) override;
    private:
        CreateProjectCallback m_Callback;
        bool m_Open = false;
    };
}
