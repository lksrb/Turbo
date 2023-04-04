#include "tbopch.h"
#include "SceneHierarchyPanel.h"

#include "Turbo/Audio/Audio.h"
#include "Turbo/Renderer/Texture2D.h"
#include "Turbo/Script/Script.h"
#include "Turbo/UI/UI.h"

#include <glm/gtc/type_ptr.hpp>

#pragma region Defines

#define TBO_TYPEFUNC(debug_name, TYPE, UI_FUNC, ...)        \
[](const std::string& name, ScriptFieldInstance& instance)  \
{                                                           \
     TYPE data = instance.GetValue<TYPE>();                 \
     if(UI_FUNC(name.c_str(), &data, __VA_ARGS__))          \
     {                                                      \
         instance.SetValue<TYPE>(data);                     \
     }                                                      \
}                                                           

#define TBO_TYPEFUNC_COMPLEX(debug_name, TYPE, UI_FUNC, ...)\
[](const std::string& name, ScriptFieldInstance& instance)  \
{                                                           \
     TYPE data = instance.GetValue<TYPE>();                 \
     if(UI_FUNC(name.c_str(), &data[0], __VA_ARGS__))       \
     {                                                      \
         instance.SetValue<TYPE>(data);                     \
     }                                                      \
}                                                           

#define TBO_TYPEFUNC_EMPTY(debug_name) \
[](const std::string& name, ScriptFieldInstance& instance) {}

#define TBO_TYPEFUNC2_EMPTY(debug_name) \
[](const std::string& name, Ref<ScriptInstance>& instance) {}

#define TBO_TYPEFUNC2(debug_name, TYPE, UI_FUNC, ...)       \
[](const std::string& name, Ref<ScriptInstance>& instance)  \
{                                                           \
    TYPE data = instance->GetFieldValue<TYPE>(name);        \
    if (UI_FUNC(name.c_str(), &data, __VA_ARGS__))          \
    {                                                       \
        instance->SetFieldValue<TYPE>(name, &data);         \
    }                                                       \
}

#define TBO_TYPEFUNC2_COMPLEX(debug_name, TYPE, UI_FUNC, ...)\
[](const std::string& name, Ref<ScriptInstance>& instance)   \
{                                                            \
    TYPE data = instance->GetFieldValue<TYPE>(name);         \
    if (UI_FUNC(name.c_str(), &data[0], __VA_ARGS__))        \
    {                                                        \
        instance->SetFieldValue<TYPE>(name, &data);          \
    }                                                        \
}    

#pragma endregion

namespace Turbo
{
    namespace Utils
    {
        template<typename T, typename UIFunction>
        static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
        {
            const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
            if (entity.HasComponent<T>())
            {
                std::string typeName(typeid(T).name());
                std::string imguiPopupID = std::string("ComponentSettings") + typeName;
                ImGui::PushID(imguiPopupID.c_str());

                auto& component = entity.GetComponent<T>();
                ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
                float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
                ImGui::Separator();
                bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
                ImGui::PopStyleVar();
                ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
                if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
                {
                    ImGui::OpenPopup("ComponentSettings");
                }

                bool removeComponent = false;
                if (ImGui::BeginPopup("ComponentSettings"))
                {
                    if (typeid(T) != typeid(TransformComponent) && ImGui::MenuItem("Remove component"))
                        removeComponent = true;

                    ImGui::EndPopup();
                }
                ImGui::PopID();
                if (open)
                {
                    uiFunction(component);
                    ImGui::TreePop();
                }

                if (removeComponent)
                {
                    entity.RemoveComponent<T>();
                }
            }
        }
        static void DrawVec3Control(const std::string& label, glm::vec3* values, float resetValue = 0.0f, float columnWidth = 100.0f)
        {
            ImGuiIO& io = ImGui::GetIO();
            auto boldFont = io.Fonts->Fonts[0];

            ImGui::PushID(label.c_str());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text(label.c_str());
            ImGui::NextColumn();

            ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

            float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
            ImGui::PushFont(boldFont);
            if (ImGui::Button("X", buttonSize))
                values->x = resetValue;
            ImGui::PopFont();
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::DragFloat("##X", &values->x, 0.1f, 0.0f, 0.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
            ImGui::PushFont(boldFont);
            if (ImGui::Button("Y", buttonSize))
                values->y = resetValue;
            ImGui::PopFont();
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::DragFloat("##Y", &values->y, 0.1f, 0.0f, 0.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
            ImGui::PushFont(boldFont);
            if (ImGui::Button("Z", buttonSize))
                values->z = resetValue;
            ImGui::PopFont();
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::DragFloat("##Z", &values->z, 0.1f, 0.0f, 0.0f, "%.2f");
            ImGui::PopItemWidth();

            ImGui::PopStyleVar();

            ImGui::Columns(1);

            ImGui::PopID();
        }

        static void CallTypeSpecificFunctionNoSceneRunning(ScriptFieldType field_type, const std::string& name, ScriptFieldInstance& instance)
        {
            static std::array<std::function<void(const std::string& name, ScriptFieldInstance& instance)>, static_cast<size_t>(ScriptFieldType::Max)> s_TypeFunctionsNSR =
            {
                TBO_TYPEFUNC("Float",  f32, ImGui::DragFloat, 0.01f),
                TBO_TYPEFUNC("Double", f32, ImGui::DragFloat, 0.01f),
                TBO_TYPEFUNC("Bool",  bool, ImGui::Checkbox),
                TBO_TYPEFUNC_EMPTY("Char"),
                TBO_TYPEFUNC("Integer", i32, UI::DragInt, 0.1f),
                TBO_TYPEFUNC("Unsigned Integer", u32, UI::DragUInt, 0.1f),
                TBO_TYPEFUNC("Short", i32, UI::DragInt, 0.1f),
                TBO_TYPEFUNC("Unsigned Short", u32, UI::DragUInt, 0.1f),
                TBO_TYPEFUNC("Long", i64, UI::DragLong, 0.1f),
                TBO_TYPEFUNC("Unsigned Long", u64, UI::DragULong, 0.1f),
                TBO_TYPEFUNC("Byte", char, UI::DragByte, 0.1f),
                TBO_TYPEFUNC("Unsigned Byte", unsigned char, UI::DragUByte, 0.1f),
                TBO_TYPEFUNC_COMPLEX("Vector2", glm::vec2, ImGui::DragFloat2, 0.1f),
                TBO_TYPEFUNC_COMPLEX("Vector3", glm::vec3, ImGui::DragFloat3, 0.1f),
                TBO_TYPEFUNC_COMPLEX("Vector4", glm::vec4, ImGui::DragFloat4, 0.1f),
            };

            u32 type = static_cast<u32>(field_type);

            // Call type specific function
            if (type < s_TypeFunctionsNSR.size())
                s_TypeFunctionsNSR[type](name, instance);

        }
        static void CallTypeSpecificFunctionSceneRunning(ScriptFieldType field_type, const std::string& name, Ref<ScriptInstance> instance)
        {
            static std::array<std::function<void(const std::string& name, Ref<ScriptInstance>& instance)>, static_cast<size_t>(ScriptFieldType::Max)> s_TypeFunctionsSR =
            {
                TBO_TYPEFUNC2("Float", f32, ImGui::DragFloat, 0.01f),
                TBO_TYPEFUNC2("Double", f32, ImGui::DragFloat, 0.01f),
                TBO_TYPEFUNC2("Bool", bool, ImGui::Checkbox),
                TBO_TYPEFUNC2_EMPTY("Char"),
                TBO_TYPEFUNC2("Integer", i32, UI::DragInt, 0.1f),
                TBO_TYPEFUNC2("Unsigned Integer", u32, UI::DragUInt, 0.1f),
                TBO_TYPEFUNC2("Short", i32, UI::DragInt, 0.1f),
                TBO_TYPEFUNC2("Unsigned Short", u32, UI::DragUInt, 0.1f),
                TBO_TYPEFUNC2("Long", i64, UI::DragLong, 0.1f),
                TBO_TYPEFUNC2("Unsigned Long", u64, UI::DragULong, 0.1f),
                TBO_TYPEFUNC2("Byte", char, UI::DragByte, 0.1f),
                TBO_TYPEFUNC2("Unsigned Byte", unsigned char, UI::DragUByte, 0.1f),
                TBO_TYPEFUNC2_COMPLEX("Vector2", glm::vec2, ImGui::DragFloat2, 0.1f),
                TBO_TYPEFUNC2_COMPLEX("Vector3", glm::vec3, ImGui::DragFloat3, 0.1f),
                TBO_TYPEFUNC2_COMPLEX("Vector4", glm::vec4, ImGui::DragFloat4, 0.1f),
            };

            u32 type = static_cast<u32>(field_type);

            TBO_ENGINE_ASSERT(type < s_TypeFunctionsSR.size());

            // Call type specific function
            if(type < s_TypeFunctionsSR.size())
                s_TypeFunctionsSR[type](name, instance);

        }
    }

    SceneHierarchyPanel::SceneHierarchyPanel()
    {
    }

    SceneHierarchyPanel::~SceneHierarchyPanel()
    {
    }

    void SceneHierarchyPanel::OnDrawUI()
    {
        ImGui::Begin("Scene Hierarchy");

        if (m_Context)
        {
            m_Context->EachEntity([&](Entity entity)
            {
                DrawEntityNode(entity);
            });

            if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
                m_SelectedEntity = {};

            // Right-click on blank space
            if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
            {
                if (ImGui::MenuItem("Create Empty Entity"))
                    m_SelectedEntity = m_Context->CreateEntity("Empty Entity");

                ImGui::EndPopup();
            }
            ImGui::End();

            ImGui::Begin("Properties");
            if (m_SelectedEntity)
            {
                DrawComponents(m_SelectedEntity);

                // Drag & drop
                ImGuiWindow* window = ImGui::GetCurrentWindow();
                ImRect window_content = window->ContentRegionRect;

                // Handle scrolling
                window_content.Max.y = window->ContentRegionRect.Max.y + window->Scroll.y;
                window_content.Min.y = window->ContentRegionRect.Min.y + window->Scroll.y;

                // Add script component if dragged into properties
                if (ImGui::BeginDragDropTargetCustom(window_content, window->ID))
                {
                    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_SHP");

                    if (!m_SelectedEntity.HasComponent<ScriptComponent>() && payload)
                    {
                        const auto& path = m_AssetsPath / (const wchar_t*)payload->Data;

                        m_SelectedEntity.AddComponent<ScriptComponent>(path.string());
                    }

                    ImGui::EndDragDropTarget();
                }
            }
        }

        ImGui::End();
    }

    void SceneHierarchyPanel::OnSceneContextChanged(const Ref<Scene>& context)
    {
        m_Context = context;

        // Reset selected entity
        SetSelectedEntity();
    }

    void SceneHierarchyPanel::OnProjectChanged(const Ref<Project>& project)
    {
        m_AssetsPath = Project::GetAssetsPath();
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().Tag;

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
            {
                tag = buffer;
            }
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(-1);

        if (ImGui::Button("Add Component"))
            ImGui::OpenPopup("AddComponent");

        if (ImGui::BeginPopup("AddComponent"))
        {
            DisplayAddComponentEntry<CameraComponent>("Camera");
            DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
            DisplayAddComponentEntry<CircleRendererComponent>("Circle Renderer");
            DisplayAddComponentEntry<AudioSourceComponent>("Audio Source Component");
            DisplayAddComponentEntry<AudioListenerComponent>("Audio Listener Component");
            DisplayAddComponentEntry<TextComponent>("Text Component");
            DisplayAddComponentEntry<BoxCollider2DComponent>("Box Collider 2D");
            DisplayAddComponentEntry<Rigidbody2DComponent>("Rigid Body 2D");
            DisplayAddComponentEntry<CircleCollider2DComponent>("Circle Collider 2D");
            DisplayAddComponentEntry<ScriptComponent>("Script Component");

            ImGui::EndPopup();
        }

        ImGui::PopItemWidth();

        Utils::DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
        {
            Utils::DrawVec3Control("Translation", &component.Translation);
            glm::vec3 rotation = glm::degrees(component.Rotation);
            Utils::DrawVec3Control("Rotation", &rotation);
            component.Rotation = glm::radians(rotation);
            Utils::DrawVec3Control("Scale", &component.Scale, 1.0f);
        });

        Utils::DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
        {
            auto& camera = component.Camera;

            ImGui::Checkbox("Primary", &component.IsPrimary);

            const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
            const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
            if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
            {
                for (i32 i = 0; i < 2; ++i)
                {
                    bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
                    if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
                    {
                        currentProjectionTypeString = projectionTypeStrings[i];
                        camera.SetProjectionType((SceneCamera::ProjectionType)i);
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
            {
                float perspectiveVerticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
                if (ImGui::DragFloat("Vertical FOV", &perspectiveVerticalFov))
                    camera.SetPerspectiveVerticalFOV(glm::radians(perspectiveVerticalFov));

                float perspectiveNear = camera.GetPerspectiveNearClip();
                if (ImGui::DragFloat("Near", &perspectiveNear))
                    camera.SetPerspectiveNearClip(perspectiveNear);

                float perspectiveFar = camera.GetPerspectiveFarClip();
                if (ImGui::DragFloat("Far", &perspectiveFar))
                    camera.SetPerspectiveFarClip(perspectiveFar);
            }

            if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
            {
                float orthoSize = camera.GetOrthographicSize();
                if (ImGui::DragFloat("Size", &orthoSize))
                    camera.SetOrthographicSize(orthoSize);

                float orthoNear = camera.GetOrthographicNearClip();
                if (ImGui::DragFloat("Near", &orthoNear))
                    camera.SetOrthographicNearClip(orthoNear);

                float orthoFar = camera.GetOrthographicFarClip();
                if (ImGui::DragFloat("Far", &orthoFar))
                    camera.SetOrthographicFarClip(orthoFar);

                ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
            }
        });

        Utils::DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [&m_AssetsPath = m_AssetsPath](auto& component)
        {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));

            ImGui::Text("Texture");
            ImGui::NextColumn();
            ImGui::SameLine();
            const ImVec2 cursor = ImGui::GetCursorPos();
            /*   ImDrawList* drawList = ImGui::GetWindowDrawList();
               drawList->AddRectFilled(cursor, { cursor.x + 100, cursor.y + 100 }, 0xff0000ff); // red*/
            ImGui::SetCursorPos(ImVec2(cursor.x + 225.0f, cursor.y));
            ImGui::PushItemWidth(-1);
            ImGui::Button("##textureButton", ImVec2(50, 50.0f));
            ImGui::PopItemWidth();
            ImGui::NextColumn();
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const char* path = (const char*)payload->Data;
                    const std::filesystem::path& texturePath = m_AssetsPath / path;

                    auto& fileExtension = texturePath.extension();

                    // Currently accepting only .pngs and .jpgs
                    bool success = fileExtension == ".png" || fileExtension == ".jpg";

                    if (success)
                    {
                        Ref<Texture2D> texture = Texture2D::Create({ texturePath.string() });
                        if (texture->IsLoaded())
                            component.Texture = texture;
                        else
                            TBO_WARN("Could not load texture {0}", texturePath.stem().string());
                    }
                    else
                        TBO_WARN("Could not load texture {0} - Invalid format", texturePath.stem().string());
                }
                ImGui::EndDragDropTarget();
            }

            // TODO(Urby): Style ImGui and make id generator for imgui id system

            ImGui::Text("Tiling");
            ImGui::NextColumn();
            ImGui::SameLine();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##tilingDragFloat", &component.Tiling, 0.1f, 0.0f, 100.0f);
            ImGui::PopItemWidth();

        });

        Utils::DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto& component)
        {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
            ImGui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
            ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
        });

        Utils::DrawComponent<TextComponent>("Text Component", entity, [&entity, m_Context = m_Context](auto& component)
        {
            ImGui::InputTextMultiline("Text", &component.Text);
            ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
            ImGui::InputFloat("Kerning Offset", &component.KerningOffset, 0.025f);
            ImGui::InputFloat("Line Spacing", &component.LineSpacing, 0.025f);
        });

        Utils::DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component)
        {
            const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
            const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];
            if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
            {
                for (int i = 0; i < 2; i++)
                {
                    bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
                    if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
                    {
                        currentBodyTypeString = bodyTypeStrings[i];
                        component.Type = (Rigidbody2DComponent::BodyType)i;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }
            ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
        });
        Utils::DrawComponent<AudioSourceComponent>("Audio Source Component", entity, [](auto& component)
        {
            static bool s_IsValidAudioFile = true; // FIXME: static in class is not ideal
            
            // Audio clip
            char s_AudioSourcePath[128];
            bool changed = false;
            memset(s_AudioSourcePath, 0, sizeof(s_AudioSourcePath));

            const auto& clipPath = component.Clip->GetFilepath();
            if (component.Clip && !changed && s_IsValidAudioFile)
            {
                strncpy_s(s_AudioSourcePath, sizeof(s_AudioSourcePath), clipPath.c_str(), sizeof(s_AudioSourcePath));
            }

            {
                UI::ScopedStyleColor textColor(ImGuiCol_Text, { 0.9f, 0.2f, 0.3f, 1.0f }, !s_IsValidAudioFile);
                ImGui::InputText("Clip Path", s_AudioSourcePath, sizeof(s_AudioSourcePath));

                changed = strcmp(s_AudioSourcePath, clipPath.c_str()) != 0;

                s_IsValidAudioFile = std::filesystem::exists(s_AudioSourcePath)
                    && std::filesystem::path(s_AudioSourcePath).extension() == ".wav";

                if (changed && s_IsValidAudioFile && ImGui::Button("Load"))
                {
                    component.Clip = Audio::CreateAndRegisterClip(s_AudioSourcePath);
                }
            }

            ImGui::DragFloat("Gain", &component.Gain, 0.05f, 0.0f, 10.0f);
            ImGui::Checkbox("Spatial", &component.Spatial);
            ImGui::Checkbox("PlayOnStart", &component.PlayOnStart);
            ImGui::Checkbox("Loop", &component.Loop);
        });

        Utils::DrawComponent<AudioListenerComponent>("Audio Listener Component", entity, [](auto& component)
        {
            ImGui::Checkbox("Primary", &component.IsPrimary);
        });

        Utils::DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
        {
            ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
            ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
            ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
        });

        Utils::DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component)
        {
            ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
            ImGui::DragFloat("Radius", &component.Radius);
            ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
        });

        Utils::DrawComponent<ScriptComponent>("Script Component", entity, [&entity, m_Context = m_Context](auto& component)
        {
            static char s_ClassNameBuffer[64];

            UUID entityUUID = entity.GetUUID();
            bool entityClassExists = Script::ScriptClassExists(component.ClassName);
            strcpy_s(s_ClassNameBuffer, sizeof(s_ClassNameBuffer), component.ClassName.c_str());

            UI::ScopedStyleColor text_color(ImGuiCol_Text, { 0.9f, 0.2f, 0.3f, 1.0f }, !entityClassExists);

            if (ImGui::InputText("Class", s_ClassNameBuffer, sizeof(s_ClassNameBuffer)))
            {
                component.ClassName = s_ClassNameBuffer;
                return;
            }

            // Fields
            if (m_Context->IsRunning())
            {
                Ref<ScriptInstance> instance = Script::FindEntityInstance(entityUUID);

                if (instance)
                {
                    auto& fields = instance->GetScriptClass()->GetFields();

                    for (const auto& [name, field] : fields)
                    {
                        Utils::CallTypeSpecificFunctionSceneRunning(field.Type, name, instance);
                    }
                }
            }
            else // Scene is not running
            {
                if (entityClassExists)
                {
                    Ref<ScriptClass> entityClass = Script::FindEntityClass(component.ClassName);

                    const auto& fields = entityClass->GetFields();

                    auto& entity_fields = Script::GetEntityFieldMap(entityUUID);

                    for (auto& [name, field] : fields)
                    {
                        ScriptFieldInstance& field_instance = entity_fields[name];
                        Utils::CallTypeSpecificFunctionNoSceneRunning(field.Type, name, field_instance);
                    }
                }
            }
        });
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& tag = entity.GetComponent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

        if (ImGui::IsItemClicked())
        {
            SetSelectedEntity(entity);
        }

        bool entityDeleted = false;
        if (ImGui::BeginPopupContextItem(0, 1))
        {
            if (ImGui::MenuItem("Delete Entity"))
                entityDeleted = true;

            ImGui::EndPopup();
        }

        if (opened)
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
            bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
            if (opened)
                ImGui::TreePop();
            ImGui::TreePop();
        }

        if (entityDeleted)
        {
            m_Context->DestroyEntity(entity);
            if (m_SelectedEntity == entity)
                m_SelectedEntity = {};
        }
    }

    template<typename T>
    void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName)
    {
        if (m_SelectedEntity.HasComponent<T>() == false)
        {
            if (ImGui::MenuItem(entryName.c_str()))
            {
                m_SelectedEntity.AddComponent<T>();
                ImGui::CloseCurrentPopup();
            }
        }
    }

}
