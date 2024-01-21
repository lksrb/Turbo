#include "tbopch.h"
#include "VisualScriptEngine.h"

#include "Turbo/Scene/Entity.h"
#include "Turbo/Scene/Components.h"

#include "Stack.h"

#define DEFINE_FUNC(type, func) s_Data.ScriptFunctions[static_cast<u32>(type)] = func
#define GET_INPUT_OR_DEFAULT_VALUE(__index, __type, __default) self.Inputs[__index].Connected.size() ? stack[self.Inputs[__index].Connected.front()->DataIndex]->As<__type>() : __default

#include <yaml-cpp/yaml.h>

namespace Turbo {

    using ScriptFunction = void(*)(ScriptNode& self, Stack& stack);
    using ScriptFunctionArray = ScriptFunction[static_cast<u32>(NodeType::Count)];

    struct VisualScriptEngineData
    {
        std::list<ScriptNode> Nodes;
        std::unordered_map<UUID, ScriptPin*> Pins;
        std::unordered_map<UUID, ScriptPinLink> Links;
        ScriptFunction ScriptFunctions[static_cast<u32>(NodeType::Count)];

        Scene* CurrentScene = nullptr;
        Stack DataStack;
    };

    static VisualScriptEngineData s_Data;

    void VisualScriptEngine::Init()
    {
        DEFINE_FUNC(NodeType::Start, [](ScriptNode& self, Stack& stack)
        {
            /* Empty */
        });

        DEFINE_FUNC(NodeType::SetTranslation, [](ScriptNode& self, Stack& stack)
        {
            Entity defaultEntity = s_Data.CurrentScene->FindEntityByUUID(stack[0]->As<UUID>());

            Entity entity = defaultEntity;
            glm::vec3 translation = self.Inputs[2].Connected.size() ? self.Inputs[2].Connected.front()->DataStorage.As<glm::vec3>() : glm::vec3(0.0f);
            entity.Transform().Translation = translation;

            // Set translation output
            self.Outputs[1].DataStorage = translation;
        });

        DEFINE_FUNC(NodeType::Vector3Literal, [](ScriptNode& self, Stack& stack)
        {
        });

        DEFINE_FUNC(NodeType::EntityLiteral, [](ScriptNode& self, Stack& stack)
        {
        });

        DEFINE_FUNC(NodeType::AddValues, [](ScriptNode& self, Stack& stack)
        {
            glm::vec3 vec1 = GET_INPUT_OR_DEFAULT_VALUE(0, glm::vec3, glm::vec3(0.0f));
            glm::vec3 vec2 = GET_INPUT_OR_DEFAULT_VALUE(1, glm::vec3, glm::vec3(0.0f));

            self.Outputs[0].DataIndex = stack.Push(vec1 + vec2);
        });

        s_Data.ScriptFunctions[static_cast<u32>(NodeType::Print)] = [](ScriptNode& self, Stack& stack)
        {
            if (self.Inputs[1].Connected.empty()) // Error!
            {
                printf("ERROR! PrintNode does not have any value to print.\n");
                return;
            }

            auto valuePin = self.Inputs[1].Connected.front();

            switch (valuePin->Type)
            {
                case ValueType::Integer:
                {
                    i32 value = stack[valuePin->DataIndex]->As<i32>();
                    printf("%d\n", value);
                    break;
                }
                case ValueType::Float:
                {
                    f32 value = stack[valuePin->DataIndex]->As<f32>();
                    printf("%f\n", value);
                    break;
                }
                case ValueType::Vector2:
                {
                    glm::vec2 value = stack[valuePin->DataIndex]->As<glm::vec2>();
                    printf("glm::vec2[%.3f, %.3f]\n", value.x, value.y);
                    break;
                }
                case ValueType::Vector3:
                {
                    glm::vec3 value = stack[valuePin->DataIndex]->As<glm::vec3>();
                    printf("glm::vec3[%.3f, %.3f, %.3f]\n", value.x, value.y, value.z);
                    break;
                }
                case ValueType::Vector4:
                {
                    glm::vec4 value = stack[valuePin->DataIndex]->As<glm::vec4>();
                    printf("glm::vec4[%.3f, %.3f, %.3f, %.3f]\n", value.x, value.y, value.z, value.w);
                    break;
                }
                case ValueType::Entity:
                {
                    assert(false);
                    break;
                }
                default:
                    assert(false);
                    break;
            }
        };

        DEFINE_FUNC(NodeType::ForLoop, [](ScriptNode& self, Stack& stack)
        {
            i32 min = GET_INPUT_OR_DEFAULT_VALUE(1, i32, 0);
            i32 max = GET_INPUT_OR_DEFAULT_VALUE(2, i32, 10);
            i32 step = GET_INPUT_OR_DEFAULT_VALUE(3, i32, 1);
            ScriptNode* bodyNode = nullptr;

            if (auto& pins = self.Outputs[1].Connected; pins.size())
            {
                bodyNode = pins.front()->Current;
            }

            // Push first index
            i32 index = self.Outputs[2].DataIndex = stack.PushEmpty();

            if (bodyNode)
            {
                for (i32 i = min; i < max; i += step)
                {
                    // Modify index each evaluation
                    *stack[index] = i;
                    Evaluate(bodyNode);
                }
            }
            else // For loop has nothing in its body - just set index value to the maximum
            {
                *stack[index] = max - step;
            }
        });
    }

    void VisualScriptEngine::Shutdown()
    {
        s_Data.Nodes.clear();
    }

    void VisualScriptEngine::Call(ScriptNode* node)
    {
        s_Data.ScriptFunctions[static_cast<u32>(node->Type)](*node, s_Data.DataStack);
    }

    void VisualScriptEngine::EntityOnStart(Entity entity)
    {
        s_Data.DataStack.Push(entity.GetUUID());
        Evaluate(&s_Data.Nodes.front());
        s_Data.DataStack.Reset();
    }

    void VisualScriptEngine::Evaluate(ScriptNode* current)
    {
        for (auto& pin : current->Inputs)
        {
            if (pin.Type == ValueType::Event || pin.Connected.empty())
                continue;

            // Value pins can only have 1 value in them
            ScriptNode* next = pin.Connected.front()->Current;

            // Inputs cannot be evaluated twice
            if (!next->Evaluated)
            {
                Evaluate(next);
            }
        }

        // Some nodes require to mark them evaluated before the actual evaluation
        current->Evaluated = true;
        Call(current);

        if (ScriptNode* next = GetNextEvent(current))
        {
            Evaluate(next);
        }
    }

    void VisualScriptEngine::Connect(ScriptPin* pin0, ScriptPin* pin1)
    {
        pin0->Connected.push_back(pin1);
        pin1->Connected.push_back(pin0);

        // Generate UUID
        UUID uuid;
        s_Data.Links[uuid] = { pin0->ID, pin1->ID };
    }

	void VisualScriptEngine::Disconnect(UUID scriptLinkId)
	{
        auto it = s_Data.Links.find(scriptLinkId);

        if (it == s_Data.Links.end())
        {
            TBO_ENGINE_ASSERT(false, "Could not find link to be deleted!");
            return;
        }

        // Find pins
        auto& link = it->second;
        ScriptPin* pin0 = s_Data.Pins[link.InputPinID];
        ScriptPin* pin1 = s_Data.Pins[link.OutputPinID];

        // Erase them from connected pins
        pin0->Connected.erase(std::find(pin0->Connected.begin(), pin0->Connected.end(), pin1));
        pin1->Connected.erase(std::find(pin1->Connected.begin(), pin1->Connected.end(), pin0));

        // Finally erase the link
        s_Data.Links.erase(it);
	}

    ScriptPin* VisualScriptEngine::GetPin(UUID uuid)
    {
        auto it = s_Data.Pins.find(uuid);
        if (it == s_Data.Pins.end())
        {
            TBO_ENGINE_ERROR("Could not find the pin! {}", uuid);
            return nullptr;
        }

        return it->second;
    }

    std::list<ScriptNode>& VisualScriptEngine::GetNodes()
    {
        return s_Data.Nodes;
    }

    std::unordered_map<UUID, ScriptPinLink>& VisualScriptEngine::GetLinks()
    {
        return s_Data.Links;
    }

    ScriptNode* VisualScriptEngine::GetNextEvent(ScriptNode* current)
    {
        if (current->Outputs[0].Type == ValueType::Event)
        {
            return current->Outputs[0].Connected.size() ? current->Outputs[0].Connected.front()->Current : nullptr;
        }

        return nullptr;
    }

    void VisualScriptEngine::OnRuntimeStart(Scene* currentScene)
    {
        s_Data.CurrentScene = currentScene;
    }

    ScriptNode* VisualScriptEngine::Create(NodeType type, Entity entity)
    {
        // Create instance since creation on execution would be too expensive
        auto& scriptNode = s_Data.Nodes.emplace_back();
        scriptNode.Type = type;
        switch (type)
        {
            case NodeType::Start:
            {
                scriptNode.DebugName = "Start Node";
                scriptNode.Outputs = { ValueType::Event };
                break;
            }
            case NodeType::SetTranslation:
            {
                scriptNode.DebugName = "Set Translation";
                scriptNode.Inputs = { ValueType::Event, ValueType::Entity, ValueType::Vector3 };
                scriptNode.Outputs = { ValueType::Event, ValueType::Vector3 };
                break;
            }
            case NodeType::Vector3Literal:
            {
                scriptNode.DebugName = "Vector3 Literal";
                scriptNode.Outputs = { ValueType::Vector3 };
                break;
            }
            case NodeType::EntityLiteral:
            {
                scriptNode.DebugName = "Entity Literal";
                scriptNode.Outputs = { ValueType::Entity };
                break;
            }
            case NodeType::Print:
            {
                scriptNode.DebugName = "Print";
                scriptNode.Inputs = { ValueType::Event, ValueType::PrintableValues };
                scriptNode.Outputs = { ValueType::Event };
                break;
            }
            case NodeType::AddValues:
            {
                scriptNode.DebugName = "AddValues";
                scriptNode.Inputs = { ValueType::AdditiveValues, ValueType::AdditiveValues };
                scriptNode.Outputs = { ValueType::AdditiveValues };
                break;
            }
            case NodeType::ForLoop:
            {
                scriptNode.DebugName = "ForLoop";
                scriptNode.Inputs = { ValueType::Event, ValueType::Integer, ValueType::Integer, ValueType::Integer };
                scriptNode.Outputs = { ValueType::Event, ValueType::NodePtr, ValueType::Integer };
                break;
            }
            default:
                TBO_ENGINE_ASSERT(false); // Unknown node type!
                break;
        }

        for (auto& input : scriptNode.Inputs)
        {
            input.Current = &scriptNode;
            input.Kind = ScriptPinKind::Input;
            s_Data.Pins[input.ID] = &input;
        }

        for (auto& output : scriptNode.Outputs)
        {
            output.Current = &scriptNode;
            output.Kind = ScriptPinKind::Output;
             s_Data.Pins[output.ID] = &output;
        }

        return &scriptNode;
    }

    void VisualScriptEngine::Destroy(ScriptNode* node)
    {
        // Then remove link from your data.
        for (auto it = s_Data.Nodes.begin(); it != s_Data.Nodes.end(); ++it)
        {
            if (&(*it) == node)
            {
                s_Data.Nodes.erase(it);
                return;
            }
        }

        TBO_ENGINE_ASSERT(false, "Destroying invalid ScriptNode!");
    }

    void VisualScriptEngine::Serialize()
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "AssetType" << YAML::Value << "";
        out << YAML::Key << "VisualNodes" << YAML::Value << YAML::BeginSeq;

        // Serialize each node
        for (auto& node : s_Data.Nodes)
        {
            out << YAML::Key << "DebugName" << node.DebugName;
            out << YAML::Key << "Type" << (u32)node.Type;
        }

        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout("TestScriptFile.vscript");

        if (fout)
        {
            fout << out.c_str();
            return;
        }

        TBO_ENGINE_ASSERT(false);

    }

}
