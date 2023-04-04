#pragma once

#include "Turbo/Event/Event.h"
#include "Turbo/Solution/Project.h"

namespace Turbo
{
    class TBO_NOVTABLE EditorPanel
    {
    public:
        virtual void OnDrawUI() = 0;
        virtual void OnEvent(Event& e) {}
        virtual void OnProjectChanged(const Ref<Project>& project) {}
        virtual void OnSceneContextChanged(const Ref<Scene>& context) {}
    private:
    };
}
