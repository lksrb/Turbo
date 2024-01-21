#pragma once

#include "ScriptNode.h"
#include "Turbo/Core/UUID.h"

#include <unordered_map>
#include <list>

namespace Turbo {

    class Stack;
    class Scene;
    class Entity;

    class VisualScriptEngine
    {
    public:
        static void Init();
        static void Shutdown();
        static ScriptNode* Create(NodeType type, Entity entity);
        static void Destroy(ScriptNode* node);
        static void Call(ScriptNode* node);

        // Move to ScriptEngine
        static void EntityOnStart(Entity entity);

        static void Evaluate(ScriptNode* current);
        static void Connect(ScriptPin* pin0, ScriptPin* pin1);
        static void Disconnect(UUID scriptLinkId);

        static ScriptPin* GetPin(UUID uuid);

        static std::list<ScriptNode>& GetNodes();
        static std::unordered_map<UUID, ScriptPinLink>& GetLinks();

        // Maybe this should be in ScriptNode class
        static ScriptNode* GetNextEvent(ScriptNode* current);

        static void OnRuntimeStart(Scene* currentScene);

        static void Serialize();
    };

}
