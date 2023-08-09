#pragma once

#include "EditorPanel.h"

#include "Turbo/Core/Common.h"
#include "Turbo/Scene/Scene.h"

namespace Turbo {

    class PanelManager {
    public:
        template<typename T, typename... Args>
        Ref<T> AddPanel(Args&&... args)
        {
            static_assert(std::is_base_of<EditorPanel, T>::value, "Class must be derived from \"Panel\" base class!");

            Ref<EditorPanel>& panel = m_Panels[typeid(T).hash_code()];
            panel = Ref<T>::Create(std::forward<Args>(args)...);
            // ...
            return panel;
        }

        template<typename T>
        Ref<T> GetPanel()
        {
            static_assert(std::is_base_of<EditorPanel, T>::value, "Class must be derived from \"Panel\" base class!");

            return m_Panels.at(typeid(T).hash_code());
        }

        void OnProjectChanged(const Ref<Project>& project);
        void SetSceneContext(const Ref<Scene>& context);

        void OnDrawUI();
        void OnEvent(Event& e);
    private:
        std::map<size_t, Ref<EditorPanel>> m_Panels;
    };

}

