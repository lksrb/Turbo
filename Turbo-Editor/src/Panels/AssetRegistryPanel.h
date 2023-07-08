#pragma once

#include <Turbo/Editor/EditorPanel.h>

namespace Turbo
{
    class AssetRegistryPanel : public EditorPanel
    {
    public:
        void Open() { m_Open = true; }

        void OnDrawUI() override;
        void OnEvent(Event& e) override;
        void OnProjectChanged(const Ref<Project>& project) override;
        void OnSceneContextChanged(const Ref<Scene>& context) override;
    private:
        u32 Filter(std::string_view name);

        std::string m_Input;
        bool m_Open = false;
    };
}
