#include "tbopch.h"
#include "SceneHierarchyPanel.h"

#include "Turbo/Renderer/Texture2D.h"

#include <imgui.h>
#include <imgui_internal.h>
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

        static void DrawVec3Control(const String64& label, glm::vec3* values, float resetValue = 0.0f, float columnWidth = 100.0f)
        {
            ImGuiIO& io = ImGui::GetIO();
            auto boldFont = io.Fonts->Fonts[0];

            ImGui::PushID(label.CStr());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text(label.CStr());
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
    }

    extern Filepath g_AssetPath;

    SceneHierarchyPanel::SceneHierarchyPanel()
    {
    }

    SceneHierarchyPanel::~SceneHierarchyPanel()
    {
    }

    void SceneHierarchyPanel::OnUIRender()
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

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().Tag;

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy_s(buffer, sizeof(buffer), tag.CStr(), sizeof(buffer));
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
            //DisplayAddComponentEntry<ScriptComponent>("Script Component");
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
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_MANAGER_ITEM"))
                {
                    const char* path = (const char*)payload->Data;
                    Filepath texturePath = g_AssetPath / path;

                    auto& fileExtension = texturePath.Extension();

                    // Currently accepting only .pngs and .jpgs
                    bool success = fileExtension == ".png" || fileExtension == ".jpg";

                    if (success)
                    {
                        Ref<Texture2D> texture = Texture2D::Create({ texturePath });
                        if (texture->IsLoaded())
                            component.Texture = texture;
                        else
                            TBO_WARN("Could not load texture {0}", texturePath.Filename().CStr());
                    }
                    else
                        TBO_WARN("Could not load texture {0} - Invalid format", texturePath.Filename().CStr());
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

        /*  DrawComponent<ScriptComponent>("Script Component", entity, [&entity, m_CurrentScene = m_CurrentScene](auto& component)
          {
              static char s_ClassNameBuffer[64];

              UUID entityUUID = entity.GetUUID();
              bool entityClassExists = ScriptEngine::EntityClassExists(component.ClassName);

              strcpy_s(s_ClassNameBuffer, sizeof(s_ClassNameBuffer), component.ClassName.c_str());

              if (entityClassExists == false)
                  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f));

              if (ImGui::InputText("Class", s_ClassNameBuffer, sizeof(s_ClassNameBuffer)))
              {
                  component.ClassName = s_ClassNameBuffer;
              }

              if (m_CurrentScene->IsRunning())
              {
                  auto& scriptInstance = ScriptEngine::FindEntityInstance(entityUUID);

                  if (scriptInstance)
                  {
                      auto& fields = scriptInstance->GetScriptClass()->GetFields();

                      for (auto& [name, field] : fields)
                      {
                          // TODO: function pointers?
                          if (field.Type == ClassFieldType::Float)
                          {
                              float32_t data = scriptInstance->GetFieldValue<float32_t>(name);
                              if (ImGui::DragFloat(name.c_str(), &data, 0.01f))
                              {
                                  scriptInstance->SetFieldValue<float32_t>(name, &data);
                              }
                          }
                      }
                  }
              }
              else // Scene is not running
              {
                  // TODO: Editor fields
                  if (entityClassExists)
                  {
                      Ref<ScriptClass> entityClass = ScriptEngine::FindEntityClass(component.ClassName);

                      if (entityClass)
                      {
                          const auto& fields = entityClass->GetFields();

                          auto& entityFields = ScriptEngine::GetEntityFieldMap(entityUUID);
                          for (auto& [name, field] : fields)
                          {
                              if (entityFields.find(name) != entityFields.end())
                              {
                                  ClassFieldInstance& instance = entityFields.at(name);

                                  if (field.Type == ClassFieldType::Float)
                                  {
                                      float32_t data = instance.GetValue<float32_t>();
                                      if (ImGui::DragFloat(name.c_str(), &data, 0.01f))
                                      {
                                          instance.SetValue<float32_t>(data);
                                      }
                                  }
                              }
                              else
                              {
                                  if (field.Type == ClassFieldType::Float)
                                  {
                                      float32_t data = 0.0f;
                                      if (ImGui::DragFloat(name.c_str(), &data, 0.01f))
                                      {
                                          ClassFieldInstance& fieldInstance = entityFields[name];
                                          fieldInstance.Field = field;
                                          fieldInstance.SetValue<float32_t>(data);
                                      }
                                  }
                              }
                          }
                      }
                  }
              }

              if (entityClassExists == false)
                  ImGui::PopStyleColor();
          });*/
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& tag = entity.GetComponent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.CStr());

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
            bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.CStr());
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
    void SceneHierarchyPanel::DisplayAddComponentEntry(const String64& entryName)
    {
        if (m_SelectedEntity.HasComponent<T>() == false)
        {
            if (ImGui::MenuItem(entryName.CStr()))
            {
                m_SelectedEntity.AddComponent<T>();
                ImGui::CloseCurrentPopup();
            }
        }
    }

}
