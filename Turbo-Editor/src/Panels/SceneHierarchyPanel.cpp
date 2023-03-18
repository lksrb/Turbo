#include "tbopch.h"
#include "SceneHierarchyPanel.h"

#include "Turbo/Renderer/Texture2D.h"

#include "Turbo/Script/Script.h"

#include "Turbo/UI/UI.h"

#include <glm/gtc/type_ptr.hpp>

namespace Turbo::Ed
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
                // Float
                [](const std::string& name, ScriptFieldInstance& instance)
                {
                    f32 data = instance.GetValue<f32>();
                    if (ImGui::DragFloat(name.c_str(), &data, 0.01f))
                    {
                        instance.SetValue<f32>(data);
                    }
                },
                // Double
                [](const std::string& name, ScriptFieldInstance& instance)
                {
                    f32 data = instance.GetValue<f32>(); // We convert double to float, because float is faster
                    if (ImGui::DragFloat(name.c_str(), &data, 0.01f))
                    {
                        instance.SetValue<f32>(data);
                    }
                },
                // Bool
                [](const std::string& name, ScriptFieldInstance& instance)
                {
                    bool data = instance.GetValue<bool>();
                    if (ImGui::Checkbox(name.c_str(), &data))
                    {
                        instance.SetValue<bool>(data);
                    }
                },
                // Char
                [](const std::string& name, ScriptFieldInstance& instance)
                {
                    // Nothing for char for now
                },
                // Integer
                [](const std::string& name, ScriptFieldInstance& instance)
                {
                    i32 data = instance.GetValue<i32>();
                    if (UI::DragInt(name.c_str(), &data, 0.01f))
                    {
                        instance.SetValue<i32>(data);
                    }
                },
                // Unsigned Integer
                [](const std::string& name, ScriptFieldInstance& instance)
                {
                    u32 data = instance.GetValue<u32>();
                    if (UI::DragUInt(name.c_str(), &data, 1.0f))
                    {
                        instance.SetValue<u32>(data);
                    }
                },
                // Short - behaves like int, just smaller boundaries
                [](const std::string& name, ScriptFieldInstance& instance)
                {
                    i32 data = instance.GetValue<i32>();
                    if (UI::DragInt(name.c_str(), &data, 1.0f))
                    {
                        instance.SetValue<i32>(data);
                    }
                },
                // Unsigned Short - behaves like unsigned int, just smaller boundaries
                [](const std::string& name, ScriptFieldInstance& instance)
                {
                    u32 data = instance.GetValue<u32>();
                    if (UI::DragUInt(name.c_str(), &data, 1.0f))
                    {
                        instance.SetValue<u32>(data);
                    }
                },
                // Long
                [](const std::string& name, ScriptFieldInstance& instance)
                {
                    i64 data = instance.GetValue<i64>();
                    if (UI::DragLong(name.c_str(), &data, 1.0f))
                    {
                        instance.SetValue<i64>(data);
                    }
                },
                // Unsigned Long
                [](const std::string& name, ScriptFieldInstance& instance)
                {
                    u64 data = instance.GetValue<u64>();
                    if (UI::DragULong(name.c_str(), &data, 1.0f))
                    {
                        instance.SetValue<u64>(data);
                    }
                },
                // Byte
                [](const std::string& name, ScriptFieldInstance& instance)
                {
                    char data = instance.GetValue<char>();
                    if (UI::DragByte(name.c_str(), &data, 1.0f, 5))
                    {
                        instance.SetValue<char>(data);
                    }
                },
                // Unsigned Byte
                [](const std::string& name, ScriptFieldInstance& instance)
                {
                    unsigned char data = instance.GetValue<unsigned char>();
                    if (UI::DragUByte(name.c_str(), &data, 1.0f))
                    {
                        instance.SetValue<u8>(data);
                    }
                },
                // Vector2
                [](const std::string& name, ScriptFieldInstance& instance)
                {
                    glm::vec2 data = instance.GetValue<glm::vec2>();
                    if (ImGui::DragFloat2(name.c_str(), &data[0], 0.01f))
                    {
                        instance.SetValue<glm::vec2>(data);
                    }
                },
                // Vector3
                [](const std::string& name, ScriptFieldInstance& instance)
                {
                    glm::vec3 data = instance.GetValue<glm::vec3>();
                    if (ImGui::DragFloat3(name.c_str(), &data[0], 0.01f))
                    {
                        instance.SetValue<glm::vec3>(data);
                    }
                },
                // Vector4
                [](const std::string& name, ScriptFieldInstance& instance)
                {
                    glm::vec4 data = instance.GetValue<glm::vec4>();
                    if (ImGui::DragFloat4(name.c_str(), &data[0], 0.01f))
                    {
                        instance.SetValue<glm::vec4>(data);
                    }
                },
            };

            u32 type = static_cast<u32>(field_type);

            TBO_ENGINE_ASSERT(type < s_TypeFunctionsNSR.size());

            // Call type specific function
            s_TypeFunctionsNSR[type](name, instance);
        }

        static void CallTypeSpecificFunctionSceneRunning(ScriptFieldType field_type, const std::string& name, Ref<ScriptInstance> instance)
        {
            static std::array<std::function<void(const std::string& name, Ref<ScriptInstance>& instance)>, static_cast<size_t>(ScriptFieldType::Max)> s_TypeFunctionsSR =
            {
                // Float
                [](const std::string& name, Ref<ScriptInstance>& instance)
                {
                    f32 data = instance->GetFieldValue<f32>(name);
                    if (ImGui::DragFloat(name.c_str(), &data, 0.01f))
                    {
                        instance->SetFieldValue<f32>(name, &data);
                    }
                },
                // Double
                [](const std::string& name, Ref<ScriptInstance>& instance)
                {
                    f32 data = instance->GetFieldValue<f32>(name); // We convert double to float, because float is faster
                    if (ImGui::DragFloat(name.c_str(), &data, 0.01f))
                    {
                        instance->SetFieldValue<f32>(name, &data);
                    }
                },
                // Bool
                [](const std::string& name, Ref<ScriptInstance>& instance)
                {
                    bool data = instance->GetFieldValue<bool>(name);
                    if (ImGui::Checkbox(name.c_str(), &data))
                    {
                        instance->SetFieldValue<bool>(name, &data);
                    }
                },
            };

            u32 type = static_cast<u32>(field_type);

            TBO_ENGINE_ASSERT(type < s_TypeFunctionsSR.size());

            // Call type specific function
            s_TypeFunctionsSR[type](name, instance);
        }
    }


    extern std::filesystem::path g_AssetPath;

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
                    m_Context->CreateEntity("Empty Entity");

                ImGui::EndPopup();
            }
            ImGui::End();

            ImGui::Begin("Properties");
            if (m_SelectedEntity)
            {
                DrawComponents(m_SelectedEntity);
                /*

                            // Add script component if dragged into properties
                            ImGui::Dummy(ImGui::GetContentRegionAvail()); // HOLY MOLY
                            if (ImGui::BeginDragDropTarget())
                            {
                                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_MANAGER_ITEM"))
                                {
                                    std::filesystem::path path = g_AssetPath / (const wchar_t*)payload->Data;

                                    m_SelectedEntity.AddComponent<ScriptComponent>(path.string());
                                }
                                ImGui::EndDragDropTarget();
                            }*/
            }
        }

        ImGui::End();
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
            DisplayAddComponentEntry<BoxCollider2DComponent>("Box Collider 2D");
            DisplayAddComponentEntry<CameraComponent>("Camera");
            DisplayAddComponentEntry<CircleCollider2DComponent>("Circle Collider 2D");
            //DisplayAddComponentEntry<CircleRendererComponent>("Circle Renderer");
            DisplayAddComponentEntry<Rigidbody2DComponent>("Rigid Body 2D");
            DisplayAddComponentEntry<ScriptComponent>("Script Component");
            DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");

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

            ImGui::Checkbox("Primary", &component.Primary);

            const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
            const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
            if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
            {
                for (int i = 0; i < 2; i++)
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

        Utils::DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
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
                    const std::filesystem::path& texturePath = g_AssetPath / path;

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

        /*
                DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto& component)
                {
                    ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
                    ImGui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
                    ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
                });*/

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

            if (entityClassExists == false)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f));

            if (ImGui::InputText("Class", s_ClassNameBuffer, sizeof(s_ClassNameBuffer)))
            {
                component.ClassName = s_ClassNameBuffer;
            }

            if (entityClassExists == false)
                ImGui::PopStyleColor();

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
#if 0
                        if (entity_fields.find(name) != entity_fields.end())
                        {
                            ScriptFieldInstance& field_instance = entity_fields.at(name);

                            // This is a fast way to remove all the branches
                            // Function pointers are stored in a vector
                            Utils::CallTypeSpecificFunctionNoSceneRunning(field.Type, name, field_instance);
                        }
                        else
                        {
                            if (field.Type == ScriptFieldType::Float)
                            {
                                f32 data = 0.0f;
                                if (ImGui::DragFloat(name.c_str(), &data, 0.01f))
                                {
                                    ScriptFieldInstance& field_instance = entity_fields[name];
                                    field_instance.Field = field;
                                    field_instance.SetValue<f32>(data);
                                }
                            }
                            else if (field.Type == ScriptFieldType::Bool)
                            {
                                // FIXME: Default field values
                                bool data = false;
                                if (ImGui::Checkbox(name.c_str(), &data))
                                {
                                    field_instance.Field = field;
                                    field_instance.SetValue<bool>(data);
                                }
                            }
                        }
#endif
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
