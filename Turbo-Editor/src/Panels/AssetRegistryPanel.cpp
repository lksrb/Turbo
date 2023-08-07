#include "tbopch.h"
#include "AssetRegistryPanel.h"

#include "Turbo/UI/UI.h"

#include <misc/cpp/imgui_stdlib.h>

#define TBO_RESULTS_ID 6479874

namespace Turbo {

    struct FilterResult
    {
        u32 CorrectLetters = 0;
        std::pair<AssetHandle, AssetMetadata> AssetData;

        bool operator<(const FilterResult& other) const
        {
            return CorrectLetters > other.CorrectLetters;
        }
    };

    AssetRegistryPanel::AssetRegistryPanel(const Callback& callback)
        : m_OpenAssetEditorCallback(callback)
    {
    }

    AssetRegistryPanel::~AssetRegistryPanel()
    {
    }

    void AssetRegistryPanel::OnDrawUI()
    {
        static AssetHandle s_SelectedHandle = 0; // TODO: If the asset is removed, this must be reset

        if (!m_Open)
            return;

        ImGui::Begin("Asset Registry", &m_Open);
        {
            UI::ScopedStyleVar styleVar(ImGuiStyleVar_FrameRounding, 10.0f);
            UI::ScopedStyleVar styleVar1(ImGuiStyleVar_FramePadding, ImVec2(3.0f, 3.0f));
            f32 maxWidth = ImGui::GetContentRegionAvail().x;

            ImGui::PushItemWidth(maxWidth);
            if (ImGui::InputText("##input", &m_Input))
            {
                s_SelectedHandle = 0;
            }
            ImGui::PopItemWidth();
        }

        auto& assetRegistry = Project::GetActive()->GetEditorAssetRegistry()->GetRegisteredAssets();
        if (assetRegistry.size())
        {
            ImGui::BeginChild(TBO_RESULTS_ID);

            std::vector<FilterResult> filteredAssets;
            filteredAssets.reserve(assetRegistry.size());

            for (const auto& [handle, metadata] : assetRegistry)
            {
                const auto& name = metadata.FilePath.stem().string();
                u32 correctLetters = FilterName(name);

                if (correctLetters)
                {
                    FilterResult& result = filteredAssets.emplace_back();
                    result.CorrectLetters = correctLetters;
                    result.AssetData = { handle, metadata };
                }
            }

            std::sort(filteredAssets.begin(), filteredAssets.end());

            for (auto& [pririty, asset] : filteredAssets)
            {
                UI::ScopedStyleVar styleVar(ImGuiStyleVar_FramePadding, ImVec2(3.0f, 3.0f));
                auto stringAssetType = Asset::StringifyAssetType(asset.second.Type);
                const auto& assetName = asset.second.FilePath.stem().string();
                bool selected = s_SelectedHandle == asset.first;

                UI::OffsetCursorPosX(8.0f);
                if (ImGui::Selectable(assetName.c_str(), &selected))
                {
                    selected = true;
                }

                ImGui::SameLine(ImGui::GetContentRegionMax().x - 73.0f); // TODO: Make more dynamic
                ImGui::Text("%s", stringAssetType);
                if (selected)
                {
                    s_SelectedHandle = asset.first;

                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        // Open asset editor
                        m_OpenAssetEditorCallback(s_SelectedHandle);
                    }
                }
            }

            ImGui::EndChild();
        }

        ImGui::End();
    }

    u32 AssetRegistryPanel::FilterName(std::string_view name)
    {
        if (m_Input.size() > name.size())
            return 0;

        if (m_Input.size() == 0)
            return 1;

        u32 correctLetters = 0;
        u32 skippedLetters = 0;
        size_t maxLength = std::min(m_Input.size(), name.size());
        for (size_t i = 0; i < maxLength; i++)
        {
            char inputChar = std::tolower(m_Input[i]);
            char nameChar = std::tolower(name[i]);

            if (inputChar == nameChar)
            {
                correctLetters++;
            }
            else
            {
                skippedLetters++;
            }
        }

        return (correctLetters > 0 && skippedLetters == 0) ? correctLetters : 0;
    }

    void AssetRegistryPanel::OnEvent(Event& e)
    {
    }

    void AssetRegistryPanel::OnProjectChanged(const Ref<Project>& project)
    {
    }

    void AssetRegistryPanel::OnSceneContextChanged(const Ref<Scene>& context)
    {
    }

}
