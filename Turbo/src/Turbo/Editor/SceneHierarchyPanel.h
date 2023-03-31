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
        void SetSelectedEntity(Entity entity = {}) { m_SelectedEntity = entity; }

        void OnSceneContextChanged(const Ref<Scene>& scene) override;
        void OnProjectChanged(const Ref<Project>& project) override;
    private:
        void DrawComponents(Entity entity);

        template<typename T>
        void DisplayAddComponentEntry(const std::string& entryName);

        void DrawEntityNode(Entity entity);
    private:
        std::filesystem::path m_AssetsPath;
        Ref<Scene> m_Context;
        Entity m_SelectedEntity;
    };
}

