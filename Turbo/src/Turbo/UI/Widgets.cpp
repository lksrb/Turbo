#include "tbopch.h"
#include "Widgets.h"

#include "UI.h"

#include "Turbo/Asset/Asset.h"
#include "Turbo/Core/FileSystem.h"
#include "Turbo/Solution/Project.h"

#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

namespace Turbo::UI {

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

    AssetHandle Widgets::AssetSearchPopup(const char* popupName, AssetType filterType)
    {
        static std::string s_InputText;
        static AssetHandle s_SelectedHandle = 0;

        struct FilterResult {
            u32 CorrectLetters = 0;
            std::pair<AssetHandle, AssetMetadata> AssetData;

            bool operator<(const FilterResult& other) const
            {
                return CorrectLetters > other.CorrectLetters;
            }
        };

        AssetHandle confirmedHandle = 0;

        if (ImGui::BeginPopup(popupName))
        {
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

                    if (correctLetters && metadata.Type == filterType)
                    {
                        FilterResult& result = filteredAssets.emplace_back();
                        result.CorrectLetters = correctLetters;
                        result.AssetData = { handle, metadata };
                    }
                }

                std::sort(filteredAssets.begin(), filteredAssets.end());

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
                        confirmedHandle = s_SelectedHandle;
                        s_SelectedHandle = 0;
                        ImGui::CloseCurrentPopup();
                        break;
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
        }
        else // Reset
        {
            s_SelectedHandle = 0;
            s_InputText = "";
        }

        return confirmedHandle;
    }

    bool Widgets::YesNoPopup(const char* popupName, const char* text)
    {
        bool yesno = false;

        if (ImGui::BeginPopup(popupName))
        {
            ImGui::Text("%s", text);
            ImGui::NextColumn();
            if (ImGui::Button("Yes"))
            {
                yesno = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("No"))
            {
                yesno = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        return yesno;
    }
#if 0
    void Widgets::CreateMeshPopup(const char* popupName, DefaultAsset defaultAsset, const CreateMeshPopupFunc& func)
    {
        auto window = ImGui::GetCurrentWindow();
        const ImGuiViewport* viewport = window->WasActive ? window->Viewport : ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(208, 88));
        if (UI::BeginPopupModal(popupName, ImGuiWindowFlags_NoResize))
        {
            static std::string s_AssetName;
            constexpr f32 offset = 2.926f;
            bool pathExists = FileSystem::Exists(Project::GetAssetsPath() / std::format("Meshes/{}.tmesh", s_AssetName));
            ImGui::Text("Name: ");
            ImGui::SameLine();
            // This will stop SetKeyboardFocusHere from grabbing focus from other items
            if (ImGui::GetCurrentWindow()->Appearing)
                ImGui::SetKeyboardFocusHere();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::InputText("##AssetName", &s_AssetName);

            UI::OffsetCursorPosX(5);
            ImVec2 framePadding = ImGui::GetStyle().FramePadding;
            ImVec2 createTextSize = ImGui::CalcTextSize("Create");
            ImVec2 cancelTextSize = ImGui::CalcTextSize("Cancel");

            f32 availableWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX(availableWidth - cancelTextSize.x - createTextSize.x - ImGui::GetStyle().ItemSpacing.x - 2 * framePadding.x);
            UI::OffsetCursorPosY(10);
            if (ImGui::Button("Cancel", ImVec2(cancelTextSize.x + 2 * framePadding.x, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
            {
                s_AssetName.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            ImGui::BeginDisabled(pathExists);
            if (ImGui::Button("Create", ImVec2(createTextSize.x + 2 * framePadding.x, 0)) || (!pathExists && ImGui::IsKeyPressed(ImGuiKey_Enter)))
            {
                Ref<Asset> asset = Project::GetActive()->GetEditorAssetRegistry()->CreateFromDefaultAsset(s_AssetName, defaultAsset);
                if (asset)
                {
                    func(s_AssetName, asset);
                }

                s_AssetName.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndDisabled();

            UI::EndPopupModal();
        }
    }
#endif

}
