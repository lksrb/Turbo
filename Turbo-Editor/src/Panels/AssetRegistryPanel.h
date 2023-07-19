#pragma once

#include <Turbo/Editor/EditorPanel.h>
#include <Turbo/Asset/Asset.h>

namespace Turbo
{

    class AssetRegistryPanel : public EditorPanel
    {
    public:
        using Callback = std::function<void(AssetHandle)>;

        AssetRegistryPanel(const Callback& callback);
        ~AssetRegistryPanel();

        void Open() { m_Open = true; }

        void OnDrawUI() override;
        void OnEvent(Event& e) override;
        void OnProjectChanged(const Ref<Project>& project) override;
        void OnSceneContextChanged(const Ref<Scene>& context) override;
    private:
        u32 FilterName(std::string_view name);

        Callback m_OpenAssetEditorCallback;

        std::string m_Input;
        bool m_Open = false;
    };
}
