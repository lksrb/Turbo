#include "tbopch.h"
#include "SceneHierarchyPanel.h"

#include "Turbo/Core/Platform.h"
#include "Turbo/Core/FileSystem.h"

#include "Turbo/Audio/Audio.h"
#include "Turbo/Renderer/Mesh.h"
#include "Turbo/Asset/AssetManager.h"
#include "Turbo/Renderer/Texture.h"
#include "Turbo/Script/Script.h"
#include "Turbo/UI/UI.h"
#include "Turbo/UI/Widgets.h"

#include <glm/gtc/type_ptr.hpp>

#include <IconsFontAwesome6.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

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

namespace Turbo {

    namespace Utils {

        template<typename T, typename UIFunction>
        static void DrawComponent(const char* name, Entity entity, UIFunction uiFunction)
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
                bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name);
                ImGui::PopStyleVar();
                ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);

                if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
                {
                    ImGui::OpenPopup("ComponentSettings");
                }

                bool removeComponent = false;
                if (ImGui::BeginPopup("ComponentSettings"))
                {
                    if (ImGui::MenuItem("Remove component") && typeid(T) != typeid(TransformComponent))
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
        static void DrawVec3Control(const char* label, glm::vec3* values, float resetValue = 0.0f, float columnWidth = 100.0f)
        {
            ImGuiIO& io = ImGui::GetIO();
            auto boldFont = io.Fonts->Fonts[0];

            ImGui::PushID(label);

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text(label);
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

        static void CallTypeSpecificFunctionNoSceneRunning(ScriptFieldType fieldType, const std::string& name, ScriptFieldInstance& instance, const Ref<Scene>& scene)
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
#if 0
                [&scene](const std::string& name, ScriptFieldInstance& instance)
                {
                    static std::string buffer = "No entity";
                    u64 uuid = instance.GetValue<u64>();
                    Entity entity = scene->FindEntityByUUID(uuid);

                    if (ImGui::InputText(name.c_str(), &buffer))
                    {
                        uuid = 0;

                        bool isNumber = true;
                        for (auto c : buffer)
                        {
                            if (!std::isdigit(c))
                            {
                                isNumber = false;
                                break;
                            }
                        }

                        if (isNumber && !buffer.empty())
                        {
                            u64 bufUUID = std::stoull(buffer);
                            entity = scene->FindEntityByUUID(bufUUID);

                            if (entity)
                            {
                                uuid = bufUUID;
                            }
                        }

                        instance.SetValue<u64>(uuid);
                    }

                    buffer = uuid ? entity.GetName() : "No entity";

                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHP_DATA"))
                        {
                            Entity entity = *(Entity*)payload->Data;
                            instance.SetValue<u64>(entity.GetUUID());

                            u64 uuid = instance.GetValue<u64>();
                            buffer = uuid ? entity.GetName() : "No entity";
                        }

                        ImGui::EndDragDropTarget();
                    }

                }
#endif
            };

            if (fieldType == ScriptFieldType::Entity)
                return;

            u32 type = static_cast<u32>(fieldType);

            // Call type specific function
            if (type < s_TypeFunctionsNSR.size())
                s_TypeFunctionsNSR[type](name, instance);

        }
        static void CallTypeSpecificFunctionSceneRunning(ScriptFieldType fieldType, const std::string& name, Ref<ScriptInstance> instance)
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
#if 0
                [](const std::string& name, Ref<ScriptInstance>& instance)
                {
                    static std::string buffer;
                    u64 scriptInstancePointer = instance->GetFieldValue<u64>(name);

                    UUID uuid = Script::GetUUIDFromMonoObject((MonoObject*)scriptInstancePointer);
                    buffer = uuid ? std::to_string(uuid) : "No entity";
                    ImGui::InputText(name.c_str(), &buffer, ImGuiInputTextFlags_ReadOnly);
                }
#endif
            };

            if (fieldType == ScriptFieldType::Entity)
                return;

            u32 type = static_cast<u32>(fieldType);

            // Call type specific function
            if (type < s_TypeFunctionsSR.size())
                s_TypeFunctionsSR[type](name, instance);
        }

        static AssetHandle GetOrLoadDefaultAsset(std::string_view meshSource, std::string_view mesh)
        {
            //constexpr std::string_view meshSource = "Sources/Default/Cube.fbx";
            //constexpr std::string_view mesh = "Meshes/Default/Cube.tmesh";
            auto assetRegistry = Project::GetActive()->GetEditorAssetRegistry();

            if (assetRegistry->IsAssetImported(meshSource))
            {
                return assetRegistry->ImportAsset(Project::GetAssetsPath() / mesh);
            }
            else // Not imported
            {
                // We need to import source asset and then create actual mesh asset
                AssetHandle sourceHandle = assetRegistry->ImportAsset(Project::GetAssetsPath() / meshSource);
                return assetRegistry->CreateAsset<StaticMesh>(Project::GetAssetsPath() / mesh, sourceHandle)->Handle;
            }
        }

#define TBO_GET_OR_CREATE_DEFAULT_ASSET(name) Utils::GetOrLoadDefaultAsset("Sources/Default/" name ".fbx", "Meshes/Default/" name ".tmesh")
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
            auto view = m_Context->GetAllEntitiesWith<RelationshipComponent>();
            for (auto e : view)
            {
                Entity entity = { e, m_Context.Get() };
                const auto& relationShipComponent = view.get<RelationshipComponent>(e);

                // If entity is root, then do draw
                if (relationShipComponent.Parent == 0)
                    DrawEntityNode(entity);
            }

            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
                m_SelectedEntity = {};

            // Right-click on blank space
            ImGui::SetNextWindowSize(ImVec2(175, 0), ImGuiCond_Always);
            if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
            {
                if (ImGui::MenuItem("Create Empty"))
                {
                    m_SelectedEntity = m_Context->CreateEntity();
                    m_SetFocusKeyboard = true;
                }

                if (ImGui::BeginMenu("2D"))
                {
                    if (ImGui::MenuItem("Sprite"))
                    {
                        m_SelectedEntity = m_Context->CreateEntity("Sprite");
                        m_SelectedEntity.AddComponent<SpriteRendererComponent>();
                        m_SetFocusKeyboard = true;
                    }

                    if (ImGui::MenuItem("Circle"))
                    {
                        m_SelectedEntity = m_Context->CreateEntity("Circle");
                        m_SelectedEntity.AddComponent<CircleRendererComponent>();
                        m_SetFocusKeyboard = true;
                    }

                    if (ImGui::MenuItem("Line"))
                    {
                        m_SelectedEntity = m_Context->CreateEntity("Line");
                        m_SelectedEntity.AddComponent<LineRendererComponent>();
                        m_SetFocusKeyboard = true;
                    }

                    if (ImGui::MenuItem("Text"))
                    {
                        // TODO: Font assets
                        //m_SelectedEntity = m_Context->CreateEntity("Text");
                        //m_SelectedEntity.AddComponent<TextComponent>();
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("3D"))
                {
                    if (ImGui::MenuItem("Cube"))
                    {
                        m_SelectedEntity = m_Context->CreateEntity("Cube");
                        auto& smr = m_SelectedEntity.AddComponent<StaticMeshRendererComponent>();
                        smr.Mesh = TBO_GET_OR_CREATE_DEFAULT_ASSET("Cube");
                    }

                    if (ImGui::MenuItem("Cone"))
                    {
                        m_SelectedEntity = m_Context->CreateEntity("Cone");
                        auto& smr = m_SelectedEntity.AddComponent<StaticMeshRendererComponent>();
                        smr.Mesh = TBO_GET_OR_CREATE_DEFAULT_ASSET("Cone");
                    }

                    if (ImGui::MenuItem("Cylinder"))
                    {
                        m_SelectedEntity = m_Context->CreateEntity("Cylinder");
                        auto& smr = m_SelectedEntity.AddComponent<StaticMeshRendererComponent>();
                        smr.Mesh = TBO_GET_OR_CREATE_DEFAULT_ASSET("Cylinder");
                    }

                    if (ImGui::MenuItem("Plane"))
                    {
                        m_SelectedEntity = m_Context->CreateEntity("Plane");
                        auto& smr = m_SelectedEntity.AddComponent<StaticMeshRendererComponent>();
                        smr.Mesh = TBO_GET_OR_CREATE_DEFAULT_ASSET("Plane");
                    }

                    if (ImGui::MenuItem("Sphere"))
                    {
                        m_SelectedEntity = m_Context->CreateEntity("Sphere");
                        auto& smr = m_SelectedEntity.AddComponent<StaticMeshRendererComponent>();
                        smr.Mesh = TBO_GET_OR_CREATE_DEFAULT_ASSET("Sphere");
                    }

                    if (ImGui::MenuItem("Capsule"))
                    {
                        m_SelectedEntity = m_Context->CreateEntity("Capsule");
                        auto& smr = m_SelectedEntity.AddComponent<StaticMeshRendererComponent>();
                        smr.Mesh = TBO_GET_OR_CREATE_DEFAULT_ASSET("Capsule");
                    }

                    if (ImGui::MenuItem("Torus"))
                    {
                        m_SelectedEntity = m_Context->CreateEntity("Torus");
                        auto& smr = m_SelectedEntity.AddComponent<StaticMeshRendererComponent>();
                        smr.Mesh = TBO_GET_OR_CREATE_DEFAULT_ASSET("Torus");
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Lights"))
                {
                    if (ImGui::MenuItem("Directional Light"))
                    {
                        m_SelectedEntity = m_Context->CreateEntity("Directional light");
                        auto& transform = m_SelectedEntity.Transform();
                        transform.Rotation.x = glm::radians(-90.0f);
                        m_SelectedEntity.AddComponent<DirectionalLightComponent>();
                    }

                    if (ImGui::MenuItem("Point Light"))
                    {
                        m_SelectedEntity = m_Context->CreateEntity("Point light");
                        m_SelectedEntity.AddComponent<PointLightComponent>();
                    }

                    if (ImGui::MenuItem("Spotlight"))
                    {
                        m_SelectedEntity = m_Context->CreateEntity("Spotlight");
                        m_SelectedEntity.Transform().Rotation.x = glm::radians(-90.0f);
                        m_SelectedEntity.AddComponent<SpotLightComponent>();
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::MenuItem("Camera"))
                {
                    m_SelectedEntity = m_Context->CreateEntity("Camera");
                    m_SelectedEntity.AddComponent<CameraComponent>();
                }

                ImGui::EndPopup();
            }

            ImGui::End();

            ImGui::Begin("Properties");

            if (m_SelectedEntity)
            {
                DrawComponents(m_SelectedEntity);

                // Add script component if dragged into properties
                if (UI::BeginDragDropTargetWindow())
                {
                    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_SHP");

                    if (!m_SelectedEntity.HasComponent<ScriptComponent>() && payload)
                    {
                        const auto& path = m_AssetsPath / (const wchar_t*)payload->Data;

                        m_SelectedEntity.AddComponent<ScriptComponent>(path.string());
                    }

                    UI::EndDragDropTargetWindow();
                }
            }
        }

        ImGui::End();
    }

    void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
    {
        m_SelectedEntity = entity;
    }

    void SceneHierarchyPanel::OnSceneContextChanged(const Ref<Scene>& context)
    {
        m_Context = context;

        // Reset selected entity
        m_SelectedEntity = {};
    }

    void SceneHierarchyPanel::OnProjectChanged(const Ref<Project>& project)
    {
        m_AssetsPath = Project::GetAssetsPath();
    }

    void SceneHierarchyPanel::OnEvent(Event& e)
    {
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().Tag;

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));

            if (m_SetFocusKeyboard)
            {
                m_SetFocusKeyboard = false;
                ImGui::SetKeyboardFocusHere();
            }
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
            {
                tag = buffer;
            }
            m_IsFocused = ImGui::IsItemActive();
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(-1);

        if (!m_Context->IsRunning() && ImGui::Button("Add Component"))
            ImGui::OpenPopup("AddComponent");

        if (ImGui::BeginPopup("AddComponent"))
        {
            DisplayAddComponentEntry<CameraComponent>("Camera");
            DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
            DisplayAddComponentEntry<LineRendererComponent>("Line Renderer");
            DisplayAddComponentEntry<CircleRendererComponent>("Circle Renderer");
            DisplayAddComponentEntry<TextComponent>("Text Component");
            DisplayAddComponentEntry<StaticMeshRendererComponent>("Static Mesh Renderer");
            DisplayAddComponentEntry<DirectionalLightComponent>("Directional Light");
            DisplayAddComponentEntry<PointLightComponent>("Point Light");
            DisplayAddComponentEntry<SpotLightComponent>("Spot Light");
            DisplayAddComponentEntry<AudioSourceComponent>("Audio Source Component");
            DisplayAddComponentEntry<AudioListenerComponent>("Audio Listener Component");
            DisplayAddComponentEntry<ScriptComponent>("Script Component");
            ImGui::Separator();
            DisplayAddComponentEntry<BoxCollider2DComponent>("Box Collider 2D");
            DisplayAddComponentEntry<Rigidbody2DComponent>("Rigid Body 2D");
            DisplayAddComponentEntry<CircleCollider2DComponent>("Circle Collider 2D");
            ImGui::Separator();
            DisplayAddComponentEntry<RigidbodyComponent>("Rigid Body");
            DisplayAddComponentEntry<BoxColliderComponent>("Box Collider");
            DisplayAddComponentEntry<SphereColliderComponent>("Sphere Collider");
            DisplayAddComponentEntry<CapsuleColliderComponent>("Capsule Collider");
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


        Utils::DrawComponent<LineRendererComponent>("Line Renderer", entity, [](auto& component)
        {
            ImGui::DragFloat3("Destination", glm::value_ptr(component.Destination), 0.5f);
            ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
        });

        Utils::DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
        {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
            ImGui::NextColumn();

            ImGui::Text("Texture: ");
            if (AssetManager::IsAssetLoaded(component.Texture))
            {
                auto& textureMetadata = AssetManager::GetAssetMetadata(component.Texture);
                auto texture = AssetManager::GetAsset<Texture2D>(component.Texture);
                component.UpdateTextureCoords(texture->GetWidth(), texture->GetHeight());

                ImGui::SameLine();
                if (ImGui::Button(textureMetadata.FilePath.stem().string().c_str()))
                {
                    ImGui::OpenPopup("SRC_YesNoPopup");
                }

                if (UI::Widgets::YesNoPopup("SRC_YesNoPopup", "Reset?"))
                {
                    component.Texture = 0;
                }
            }

            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 50.0f);

            if (ImGui::Button("Open Asset"))
                ImGui::OpenPopup("SRC_AssetSearchPopup");

            if (AssetHandle confirmedHandle = UI::Widgets::AssetSearchPopup("SRC_AssetSearchPopup", AssetType_Texture2D))
            {
                component.Texture = confirmedHandle;
            }

            ImGui::NextColumn();
            ImGui::Checkbox("Is Sprite Sheet", &component.IsSpriteSheet);

            if (component.IsSpriteSheet)
            {
                ImGui::InputFloat2("Sprite Coordinates", glm::value_ptr(component.SpriteCoords));
                ImGui::InputFloat2("Sprite Size", glm::value_ptr(component.SpriteSize));
            }

            // TODO: Style ImGui and make id generator for imgui id system

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

        Utils::DrawComponent<TextComponent>("Text Component", entity, [](auto& component)
        {
            ImGui::InputTextMultiline("Text", &component.Text);
            ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
            ImGui::InputFloat("Kerning Offset", &component.KerningOffset, 0.025f);
            ImGui::InputFloat("Line Spacing", &component.LineSpacing, 0.025f);
        });

        Utils::DrawComponent<StaticMeshRendererComponent>("Static Mesh Renderer", entity, [](auto& component)
        {
            ImGui::Text("Mesh: ");
            if (AssetManager::IsAssetLoaded(component.Mesh))
            {
                auto& meshMetadata = AssetManager::GetAssetMetadata(component.Mesh);

                ImGui::SameLine();
                if (ImGui::Button(meshMetadata.FilePath.stem().string().c_str()))
                {
                    ImGui::OpenPopup("SRC_YesNoPopup");
                }

                if (UI::Widgets::YesNoPopup("SRC_YesNoPopup", "Reset?"))
                {
                    component.Mesh = 0;
                }
            }

            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 50.0f);

            if (ImGui::Button("Open Asset"))
                ImGui::OpenPopup("SRC_AssetSearchPopup");

            if (AssetHandle confirmedHandle = UI::Widgets::AssetSearchPopup("SRC_AssetSearchPopup", AssetType_StaticMesh))
            {
                component.Mesh = confirmedHandle;
            }
        });

        Utils::DrawComponent<DirectionalLightComponent>("Directional Light", entity, [](auto& component)
        {
            ImGui::ColorEdit3("Radiance", glm::value_ptr(component.Radiance));
            ImGui::DragFloat("Intensity", &component.Intensity, 0.025f, 0.0f);
        });

        Utils::DrawComponent<PointLightComponent>("Point Light", entity, [](auto& component)
        {
            ImGui::ColorEdit3("Radiance", glm::value_ptr(component.Radiance));
            ImGui::DragFloat("Radius", &component.Radius, 0.025f, 0.0f);
            ImGui::DragFloat("Intensity", &component.Intensity, 0.025f, 0.0f);
            ImGui::DragFloat("Fall Off", &component.FallOff, 0.025f, 0.0f);
        });

        Utils::DrawComponent<SpotLightComponent>("Spot Light", entity, [](auto& component)
        {
            ImGui::ColorEdit3("Radiance", glm::value_ptr(component.Radiance));
            ImGui::InputFloat("Intensity", &component.Intensity, 0.025f);
            ImGui::InputFloat("Inner Cone", &component.InnerCone, 0.025f);
            ImGui::InputFloat("Outer Cone", &component.OuterCone, 0.025f);
        });

        Utils::DrawComponent<AudioSourceComponent>("Audio Source", entity, [&entity](auto& component)
        {
            static std::filesystem::path s_AudioSourcePath;
            static std::string s_Filename;

            s_AudioSourcePath = component.AudioPath;
            s_Filename = s_AudioSourcePath.filename().string();

            ImGui::InputText("Audio Path", &s_Filename, ImGuiInputTextFlags_ReadOnly);
            ImGui::SameLine();

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 2.0f);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f);
            if (ImGui::Button(ICON_FA_FOLDER, { 25.0f, 25.0f }))
            {
                s_AudioSourcePath = Platform::OpenFileDialog(L"Open Audio File", L"WAV File (*.wav)\0*.wav\0");
                component.AudioPath = s_AudioSourcePath.string();
                Audio::Register(entity.GetUUID(), component.AudioPath); // TOOD: Assets
            }

            ImGui::DragFloat("Gain", &component.Gain, 0.05f, 0.0f, 10.0f);
            ImGui::Checkbox("Spatial", &component.Spatial);
            ImGui::Checkbox("Play On Awake", &component.PlayOnAwake);
            ImGui::Checkbox("Loop", &component.Loop);
        });

        Utils::DrawComponent<AudioListenerComponent>("Audio Listener", entity, [](auto& component)
        {
            ImGui::Checkbox("Primary", &component.IsPrimary);
        });

        Utils::DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component)
        {
            static const char* bodyTypeStrings[] = { "Static", "Kinematic", "Dynamic" };
            const char* currentBodyTypeString = bodyTypeStrings[(u32)component.Type];
            if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
            {
                for (u32 i = 0; i < 3; ++i)
                {
                    bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
                    if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
                    {
                        currentBodyTypeString = bodyTypeStrings[i];
                        component.Type = static_cast<RigidbodyType>(i);
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }
            ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
            ImGui::Checkbox("Is Bullet", &component.IsBullet);
            ImGui::DragFloat("Gravity Scale", &component.GravityScale, 0.1f);
        });

        Utils::DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
        {
            ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
            ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
            ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
            ImGui::Checkbox("Is Trigger", &component.IsTrigger);
        });

        Utils::DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component)
        {
            ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
            ImGui::DragFloat("Radius", &component.Radius);
            ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
            ImGui::Checkbox("Is Trigger", &component.IsTrigger);
        });

        Utils::DrawComponent<ScriptComponent>("Script", entity, [&entity, m_Context = m_Context](auto& component)
        {
            if (ImGui::BeginCombo("Scripts", component.ClassName.empty() ? "<No Script>" : component.ClassName.c_str()))
            {
                const auto& scriptClassMap = Script::GetScriptClassMap();

                bool isSelected = component.ClassName.empty();
                if (ImGui::Selectable("<No Script>", isSelected))
                {
                    component.ClassName.clear();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();

                for (auto& [className, _] : scriptClassMap)
                {
                    isSelected = component.ClassName == className;
                    if (ImGui::Selectable(className.c_str(), isSelected))
                    {
                        component.ClassName = className;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            // Fields
            UUID entityUUID = entity.GetUUID();

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
                bool entityClassExists = Script::ScriptClassExists(component.ClassName);

                if (entityClassExists)
                {
                    Ref<ScriptClass> entityClass = Script::FindEntityClass(component.ClassName);

                    const auto& classFields = entityClass->GetFields();

                    auto& entityFields = Script::GetEntityFieldMap(entityUUID);

                    for (auto& [name, field] : classFields)
                    {
                        ScriptFieldInstance& fieldInstance = entityFields[name];
                        Utils::CallTypeSpecificFunctionNoSceneRunning(field.Type, name, fieldInstance, m_Context);
                    }
                }
            }
        });

        Utils::DrawComponent<RigidbodyComponent>("Rigid Body", entity, [](auto& component)
        {
            static constexpr const char* s_BodyTypeStrings[] = { "Static", "Kinematic", "Dynamic" };
            static constexpr const char* s_CollisionDetectionTypeStrings[] = { "Discrete", "LinearCast" };

            const char* currentBodyTypeString = s_BodyTypeStrings[(u32)component.Type];
            if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
            {
                for (u32 i = 0; i < 3; ++i)
                {
                    bool isSelected = currentBodyTypeString == s_BodyTypeStrings[i];
                    if (ImGui::Selectable(s_BodyTypeStrings[i], isSelected))
                    {
                        currentBodyTypeString = s_BodyTypeStrings[i];
                        component.Type = static_cast<RigidbodyType>(i);
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            const char* currentDetectionTypeString = s_CollisionDetectionTypeStrings[(u32)component.CollisionDetection];
            if (ImGui::BeginCombo("Collision Detection", currentDetectionTypeString))
            {
                for (u32 i = 0; i < 2; ++i)
                {
                    bool isSelected = currentDetectionTypeString == s_CollisionDetectionTypeStrings[i];
                    if (ImGui::Selectable(s_CollisionDetectionTypeStrings[i], isSelected))
                    {
                        currentDetectionTypeString = s_CollisionDetectionTypeStrings[i];
                        component.CollisionDetection = static_cast<CollisionDetectionType>(i);
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            ImGui::DragFloat("Gravity Scale", &component.GravityScale);
            ImGui::DragFloat("Mass", &component.Mass);
            ImGui::DragFloat("Linear Damping", &component.LinearDamping);
            ImGui::DragFloat("Angular Damping", &component.AngularDamping);

            ImGui::Checkbox("Is Trigger", &component.IsTrigger);
            ImGui::DragFloat("Friction", &component.Friction);
            ImGui::DragFloat("Restitution", &component.Restitution);

            ImGui::PushID("Lock Translation:    ");
            ImGui::Text("Lock Translation: ");
            ImGui::SameLine();
            ImGui::Checkbox("X", &component.LockTranslationX);
            ImGui::SameLine();
            ImGui::Checkbox("Y", &component.LockTranslationY);
            ImGui::SameLine();
            ImGui::Checkbox("Z", &component.LockTranslationZ);
            ImGui::PopID();

            ImGui::PushID("Lock Rotation:    ");
            ImGui::Text("Lock Rotation:    ");
            ImGui::SameLine();
            ImGui::Checkbox("X", &component.LockRotationX);
            ImGui::SameLine();
            ImGui::Checkbox("Y", &component.LockRotationY);
            ImGui::SameLine();
            ImGui::Checkbox("Z", &component.LockRotationZ);
            ImGui::PopID();
        });

        Utils::DrawComponent<BoxColliderComponent>("Box Collider", entity, [](auto& component)
        {
            ImGui::DragFloat3("Offset", glm::value_ptr(component.Offset));
            ImGui::DragFloat3("Size", glm::value_ptr(component.Size));
        });

        Utils::DrawComponent<SphereColliderComponent>("Sphere Collider", entity, [](auto& component)
        {
            // TODO: Offset in local space
            //ImGui::DragFloat3("Offset", glm::value_ptr(component.Offset));
            ImGui::DragFloat("Radius", &component.Radius);
        });

        Utils::DrawComponent<CapsuleColliderComponent>("Capsule Collider", entity, [](auto& component)
        {
            // TODO: Offset in local space
            //ImGui::DragFloat3("Offset", glm::value_ptr(component.Offset));
            ImGui::DragFloat("Radius", &component.Radius);
            ImGui::DragFloat("Height", &component.Height);
        });
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        if (!entity)
            return;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

        if (!entity.HasChildren())
            flags |= ImGuiTreeNodeFlags_Leaf;

        if (m_SelectedEntity == entity)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool opened = ImGui::TreeNodeEx((void*)(u64)(u32)entity, flags, entity.GetName().c_str());

        // TODO: Figure out how to cancel this when drag n drop is active
        if (ImGui::IsItemClicked())
        {
            SetSelectedEntity(entity);
        }

        // Drag & Drop
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHP_DATA"))
            {
                Entity childEntity = *(Entity*)payload->Data;
                if (childEntity)
                {
                    childEntity.SetParent(entity);
                }
            }

            ImGui::EndDragDropTarget();
        }

        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload("SHP_DATA", &m_SelectedEntity, sizeof(Entity), ImGuiCond_Always);
            ImGui::EndDragDropSource();
        }

        bool entityDestroyed = false;
        if (entity && ImGui::BeginPopupContextItem(0, 1))
        {
            if (ImGui::MenuItem("New Child Entity"))
            {
                Entity child = m_Context->CreateEntity();
                child.SetParent(entity);
            }

            if (ImGui::MenuItem("Destroy Entity"))
                entityDestroyed = true;

            Entity parent = entity.GetParent();
            if (parent && ImGui::MenuItem("UnParent Entity"))
            {
                //GetParent(nullptr); FIXME: Why does scene hierarchy panel includes WinUser.h ???

                entity.UnParent();
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Copy UUID"))
            {
                std::string strID = std::to_string(entity.GetUUID());
                ImGui::SetClipboardText(strID.c_str());
            }

            ImGui::EndPopup();
        }

        if (opened)
        {
            auto& children = entity.GetChildren();
            for (auto& childUUID : children)
                DrawEntityNode(m_Context->FindEntityByUUID(childUUID));

            ImGui::TreePop();
        }

        if (entityDestroyed)
        {
            m_Context->GetPostUpdateFuncs().push_back([this, entity]() { m_Context->DestroyEntity(entity); });
            if (m_SelectedEntity == entity)
                m_SelectedEntity = {};
        }
    }

    template<typename T>
    void SceneHierarchyPanel::DisplayAddComponentEntry(std::string_view entryName)
    {
        if (!m_SelectedEntity.HasComponent<T>())
        {
            if (ImGui::MenuItem(entryName.data()))
            {
                m_SelectedEntity.AddComponent<T>();
                ImGui::CloseCurrentPopup();
            }
        }
    }
}
