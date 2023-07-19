#pragma once

#include <Turbo/Asset/Asset.h>
#include <Turbo/Editor/EditorPanel.h>

namespace Turbo
{
    class AssetEditorPanel : public EditorPanel
    {
    public:
        void OnDrawUI() override;
        void OnEvent(Event& e) override;
        void OnProjectChanged(const Ref<Project>& project) override;
        void OnSceneContextChanged(const Ref<Scene>& context) override;

        void OpenAsset(AssetHandle handle);
    private:
        // TODO: Small class drawing this?
        void Texture2DAssetEditor();

        AssetHandle m_Handle = 0;
        bool m_Open = false;
    };
}
