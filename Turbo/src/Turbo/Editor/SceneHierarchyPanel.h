#pragma once

#include "EditorPanel.h"

#include "Turbo/Scene/Entity.h"
#include "Turbo/Scene/Scene.h"
#include "Turbo/Event/Event.h"
#include "Turbo/Event/KeyEvent.h"

namespace Turbo
{
    class SceneHierarchyPanel : public EditorPanel
    {
    public:
        SceneHierarchyPanel();
        ~SceneHierarchyPanel();

        void OnDrawUI() override;
        Entity GetSelectedEntity() const { return m_SelectedEntity; }
        void SetSelectedEntity(Entity entity);

        void OnSceneContextChanged(const Ref<Scene>& scene) override;
        void OnProjectChanged(const Ref<Project>& project) override;

        void OnEvent(Event& e) override;

        bool IsFocused() const { return m_IsFocused; }
    private:
        void DrawComponents(Entity entity);

        template<typename T>
        void DisplayAddComponentEntry(const std::string& entryName);

        void DrawEntityNode(Entity entity);
    private:
        bool m_IsFocused = false;

        std::filesystem::path m_AssetsPath;
        Ref<Scene> m_Context;
        Entity m_SelectedEntity;
    };
}

