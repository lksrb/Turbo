#include "tbopch.h"
#include "AssetEditorPanel.h"

#include <Turbo/Asset/AssetManager.h>

#include "Turbo/UI/UI.h"

namespace Turbo
{
    void AssetEditorPanel::OnEvent(Event& e)
    {
    }

    void AssetEditorPanel::OnProjectChanged(const Ref<Project>& project)
    {
    }

    void AssetEditorPanel::OnSceneContextChanged(const Ref<Scene>& context)
    {
    }

    void AssetEditorPanel::OnDrawUI()
    {
        if (!m_Open)
            return;

        ImGui::Begin("AssetEditorPanel", &m_Open);
        if (AssetManager::IsAssetLoaded(m_Handle))
        {
            const auto& metadata = AssetManager::GetAssetMetadata(m_Handle);
            switch (metadata.Type)
            {
                case AssetType_Texture2D: Texture2DAssetEditor(); break;
            }
        }

        ImGui::End();
    }

    void AssetEditorPanel::OpenAsset(AssetHandle handle)
    {
        m_Open = true;
        m_Handle = handle;
    }

    void AssetEditorPanel::Texture2DAssetEditor()
    {
        const auto& config = AssetManager::GetAsset<Texture2D>(m_Handle)->GetConfig();

        static const char* s_FilterValues[] = { "Nearest", "Linear" };

        const char* selected = s_FilterValues[config.Filter];

        static ImageFilter s_Filter;
        static ImageFormat s_Format;

        if (ImGui::BeginCombo("Filter", selected))
        {
            for (u32 i = 0; i < 2; i++)
            {
                if (ImGui::Selectable(s_FilterValues[i]))
                {
                    if (config.Format == i)
                        continue;

                    // TODO: Recreate asset
                }
            }

            ImGui::EndCombo();
        }

        if (ImGui::Button("Apply"))
        {

        }
    }

}
