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

        ImGui::Begin("Asset Editor", &m_Open);
        if (AssetManager::IsAssetLoaded(m_Handle))
        {
            const auto& metadata = AssetManager::GetAssetMetadata(m_Handle);
            switch (metadata.Type)
            {
                case AssetType_Texture2D: Texture2DAssetEditor(); break; // TODO: Maybe create separate classes for this
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
        auto texture = AssetManager::GetAsset<Texture2D>(m_Handle);
        auto& config = texture->GetConfig();

        static const char* s_FilterValues[ImageFilter_Count] = { "Nearest", "Linear" };
        static ImageFilter s_Filter = ImageFilter_Count;

        // Set values once
        if (s_Filter == ImageFilter_Count)
        {
            s_Filter = config.Filter;
        }

        const char* selected = s_FilterValues[s_Filter];

        if (ImGui::BeginCombo("Filter", selected))
        {
            for (u32 i = 0; i < 2; i++)
            {
                if (s_Filter == i)
                    continue;

                if (ImGui::Selectable(s_FilterValues[i]))
                {
                    s_Filter = (ImageFilter)i;
                }
            }

            ImGui::EndCombo();
        }

        // TODO: Figure out whats wrong with previews
        // They appear to ignore filtering
        //UI::Image(texture, { (f32)config.Width, (f32)config.Height }, {0,1}, {1,0});

        if (ImGui::Button("Apply"))
        {
            Texture2D::Config newConfig = config;
            newConfig.Filter = s_Filter;
            Project::GetActive()->GetEditorAssetRegistry()->RecreateAsset<Texture2D>(m_Handle, newConfig);
        }
    }

}
