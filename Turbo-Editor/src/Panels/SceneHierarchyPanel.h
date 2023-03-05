#pragma once

#include "Panel.h"

#include <Turbo/Scene/Entity.h>
#include <Turbo/Scene/Scene.h>

#include <Turbo/Event/Event.h>

namespace Turbo::Ed
{
    class SceneHierarchyPanel : public Panel
    {
    public:
        SceneHierarchyPanel();
        ~SceneHierarchyPanel();

        void OnDrawUI() override;
        void OnEvent(Event& e) override;

        void SetContext(const Ref<Scene>& context) { m_Context = context; }
        Entity GetSelectedEntity() const { return m_SelectedEntity; }
        void SetSelectedEntity(Entity entity = {}) { m_SelectedEntity = entity; }
    private:
        void DrawComponents(Entity entity);

        template<typename T>
        void DisplayAddComponentEntry(const String& entryName);

        void DrawEntityNode(Entity entity);
    private:
        Ref<Scene> m_Context;
        Entity m_SelectedEntity;
    };
}

