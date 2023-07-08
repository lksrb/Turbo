#include "tbopch.h"
#include "Widgets.h"

#include "UI.h"

#include "Turbo/Asset/Asset.h"
#include "Turbo/Solution/Project.h"

namespace Turbo
{
    static u32 FilterResults(std::string_view input, std::string_view name)
    {
        if (input.size() > name.size())
            return 0;

        if (input.size() == 0)
            return 1;

        u32 correctLetters = 0;
        u32 skippedLetters = 0;
        size_t maxLength = std::min(input.size(), name.size());
        for (size_t i = 0; i < maxLength; i++)
        {
            char inputChar = std::tolower(input[i]);
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
    };

    AssetHandle Widgets::AssetSearchPopup(const char* popupName)
    {
        static std::string s_InputText;

        struct FilterResult
        {
            u32 CorrectLetters = 0;
            std::pair<AssetHandle, AssetMetadata> AssetData;

            bool operator<(const FilterResult& other) const
            {
                return CorrectLetters > other.CorrectLetters;
            }
        };

        if (!ImGui::BeginPopup(popupName))
            return 0;

        {
            UI::ScopedStyleVar styleVar(ImGuiStyleVar_FrameRounding, 5.0f);
            UI::ScopedStyleVar styleVar1(ImGuiStyleVar_FramePadding, ImVec2(3.0f, 3.0f));
            f32 maxWidth = ImGui::GetContentRegionAvail().x;

            ImGui::PushItemWidth(maxWidth);
            ImGui::InputTextWithHint("##input", "Search...", &s_InputText);
            ImGui::PopItemWidth();
        }

        if (ImGui::BeginListBox("##SearchListBox", ImVec2(-FLT_MIN, 0.0f)))
        {
            auto& assetRegistry = Project::GetActive()->GetEditorAssetRegistry()->GetRegisteredAssets();

            std::vector<FilterResult> filteredAssets;
            filteredAssets.reserve(assetRegistry.size());

            for (const auto& [handle, metadata] : assetRegistry)
            {
                const auto& name = metadata.FilePath.stem().string();
                u32 correctLetters = FilterResults(s_InputText, name);

                if (correctLetters)
                {
                    FilterResult& result = filteredAssets.emplace_back();
                    result.CorrectLetters = correctLetters;
                    result.AssetData = { handle, metadata };
                }
            }

            std::sort(filteredAssets.begin(), filteredAssets.end());

            static AssetHandle s_SelectedHandle = 0;
            AssetMetadata selectedMetadata;

            for (auto& [pririty, asset] : filteredAssets)
            {
                UI::ScopedStyleVar styleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 1.0f));
                auto stringAssetType = Asset::StringifyAssetType(asset.second.Type);
                const auto& assetName = asset.second.FilePath.stem().string();
                bool selected = s_SelectedHandle == asset.first;

                if (ImGui::Selectable(assetName.c_str(), &selected))
                {
                    selected = true;
                }

                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && selected)
                {
                    return s_SelectedHandle;
                }
                ImGui::SameLine(); // TODO: Make more dynamic
                UI::OffsetCursorPosX(7.0f);

                UI::ScopedStyleColor textColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
                ImGui::Text("%s", stringAssetType);
                if (selected)
                {
                    s_SelectedHandle = asset.first;
                    selectedMetadata = asset.second;
                }
            }

            ImGui::EndListBox();
        }

        ImGui::EndPopup();

        return 0;
    }

}
