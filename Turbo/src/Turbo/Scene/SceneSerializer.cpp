#include "tbopch.h"
#include "SceneSerializer.h"

#include "Scene.h"
#include "Entity.h"

#include "Turbo/Audio/Audio.h"
#include "Turbo/Script/Script.h"

#include <yaml-cpp/yaml.h>

#define WRITE_SCRIPT_FIELD(FieldType, Type)           \
			case ScriptFieldType::FieldType:          \
				out << scriptField.GetValue<Type>();  \
				break

#define READ_SCRIPT_FIELD(FieldType, Type)             \
	case ScriptFieldType::FieldType:                   \
	{                                                  \
		Type data = scriptField["Data"].as<Type>();    \
		fieldInstance.SetValue(data);                  \
		break;                                         \
	}


namespace YAML
{
    template<>
    struct convert<glm::vec2>
    {
        static Node encode(const glm::vec2& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec2& rhs)
        {
            if (!node.IsSequence() || node.size() != 2)
                return false;

            rhs.x = node[0].as<Turbo::f32>();
            rhs.y = node[1].as<Turbo::f32>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<Turbo::f32>();
            rhs.y = node[1].as<Turbo::f32>();
            rhs.z = node[2].as<Turbo::f32>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            rhs.x = node[0].as<Turbo::f32>();
            rhs.y = node[1].as<Turbo::f32>();
            rhs.z = node[2].as<Turbo::f32>();
            rhs.w = node[3].as<Turbo::f32>();
            return true;
        }
    };

    template<>
    struct convert<Turbo::UUID>
    {
        static Node encode(const Turbo::UUID& uuid)
        {
            Node node;
            node.push_back(static_cast<Turbo::u64>(uuid));
            return node;
        }

        static bool decode(const Node& node, Turbo::UUID& uuid)
        {
            uuid = node.as<Turbo::u64>();
            return true;
        }
    };
}

namespace Turbo
{
    namespace Utils
    {
        static const char* ScriptFieldTypeToString(ScriptFieldType fieldType)
        {
            switch (fieldType)
            {
                case ScriptFieldType::None:    return "None";
                case ScriptFieldType::Float:   return "Float";
                case ScriptFieldType::Double:  return "Double";
                case ScriptFieldType::Bool:    return "Bool";
                case ScriptFieldType::Char:    return "Char";
                case ScriptFieldType::Byte:    return "Byte";
                case ScriptFieldType::Short:   return "Short";
                case ScriptFieldType::Int:     return "Int";
                case ScriptFieldType::Long:    return "Long";
                case ScriptFieldType::UByte:   return "UByte";
                case ScriptFieldType::UShort:  return "UShort";
                case ScriptFieldType::UInt:    return "UInt";
                case ScriptFieldType::ULong:   return "ULong";
                case ScriptFieldType::Vector2: return "Vector2";
                case ScriptFieldType::Vector3: return "Vector3";
                case ScriptFieldType::Vector4: return "Vector4";
                case ScriptFieldType::Entity:  return "Entity";
            }
            TBO_ENGINE_ASSERT(false, "Unknown ScriptFieldType");
            return "None";
        }

        static ScriptFieldType ScriptFieldTypeFromString(std::string_view fieldType)
        {
            if (fieldType == "None")    return ScriptFieldType::None;
            if (fieldType == "Float")   return ScriptFieldType::Float;
            if (fieldType == "Double")  return ScriptFieldType::Double;
            if (fieldType == "Bool")    return ScriptFieldType::Bool;
            if (fieldType == "Char")    return ScriptFieldType::Char;
            if (fieldType == "Byte")    return ScriptFieldType::Byte;
            if (fieldType == "Short")   return ScriptFieldType::Short;
            if (fieldType == "Int")     return ScriptFieldType::Int;
            if (fieldType == "Long")    return ScriptFieldType::Long;
            if (fieldType == "UByte")   return ScriptFieldType::UByte;
            if (fieldType == "UShort")  return ScriptFieldType::UShort;
            if (fieldType == "UInt")    return ScriptFieldType::UInt;
            if (fieldType == "ULong")   return ScriptFieldType::ULong;
            if (fieldType == "Vector2") return ScriptFieldType::Vector2;
            if (fieldType == "Vector3") return ScriptFieldType::Vector3;
            if (fieldType == "Vector4") return ScriptFieldType::Vector4;
            if (fieldType == "Entity")  return ScriptFieldType::Entity;

            TBO_ENGINE_ASSERT(false, "Unknown ScriptFieldType");
            return ScriptFieldType::None;
        }
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    static std::string RigidBody2DBodyTypeToString(Rigidbody2DComponent::BodyType bodyType)
    {
        switch (bodyType)
        {
            case Rigidbody2DComponent::BodyType::Static:    return "Static";
            case Rigidbody2DComponent::BodyType::Dynamic:   return "Dynamic";
            case Rigidbody2DComponent::BodyType::Kinematic: return "Kinematic";
        }

        TBO_ENGINE_ASSERT(false, "Unknown body type");
        return {};
    }

    static Rigidbody2DComponent::BodyType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString)
    {
        if (bodyTypeString == "Static")    return Rigidbody2DComponent::BodyType::Static;
        if (bodyTypeString == "Dynamic")   return Rigidbody2DComponent::BodyType::Dynamic;
        if (bodyTypeString == "Kinematic") return Rigidbody2DComponent::BodyType::Kinematic;

        TBO_ENGINE_ASSERT(false, "Unknown body type");
        return Rigidbody2DComponent::BodyType::Static;
    }

    static void SerializeEntity(YAML::Emitter& out, Entity entity)
    {
        TBO_ENGINE_ASSERT(entity.HasComponent<IDComponent>(), "Entity does not have an UUID! Something went horribly wrong!");
        UUID uuid = entity.GetUUID();

        out << YAML::BeginMap; // Entity
        out << YAML::Key << "Entity" << YAML::Value << uuid;

        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().Tag;

            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap;

            out << YAML::Key << "Tag" << YAML::Value << tag;

            out << YAML::EndMap;
        }
        if (entity.HasComponent<TransformComponent>())
        {
            auto& transform = entity.GetComponent<TransformComponent>();

            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap;

            out << YAML::Key << "Translation" << YAML::Value << transform.Translation;
            out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
            out << YAML::Key << "Scale" << YAML::Value << transform.Scale;

            out << YAML::EndMap;
        }
        if (entity.HasComponent<CameraComponent>())
        {
            out << YAML::Key << "CameraComponent";
            out << YAML::BeginMap;

            auto& cameraComponent = entity.GetComponent<CameraComponent>();
            auto& camera = cameraComponent.Camera;

            out << YAML::Key << "Camera" << YAML::Value;
            out << YAML::BeginMap;
            out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
            out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
            out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
            out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
            out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
            out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
            out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
            out << YAML::EndMap;

            out << YAML::Key << "Primary" << YAML::Value << cameraComponent.IsPrimary;
            out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

            out << YAML::EndMap;
        }

        if (entity.HasComponent<SpriteRendererComponent>())
        {
            out << YAML::Key << "SpriteRendererComponent";
            out << YAML::BeginMap;

            auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
            out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;

            if (spriteRendererComponent.Texture)
                out << YAML::Key << "TexturePath" << YAML::Value << spriteRendererComponent.Texture->GetFilepath();
            else
                out << YAML::Key << "TexturePath" << YAML::Value << "None";

            out << YAML::EndMap;
        }

        if (entity.HasComponent<CircleRendererComponent>())
        {
            out << YAML::Key << "CircleRendererComponent";
            out << YAML::BeginMap;

            auto& circleRendererComponent = entity.GetComponent<CircleRendererComponent>();
            out << YAML::Key << "Color" << YAML::Value << circleRendererComponent.Color;
            out << YAML::Key << "Thickness" << YAML::Value << circleRendererComponent.Thickness;
            out << YAML::Key << "Fade" << YAML::Value << circleRendererComponent.Fade;

            out << YAML::EndMap;
        }

        if (entity.HasComponent<TextComponent>())
        {
            out << YAML::Key << "TextComponent";
            out << YAML::BeginMap;

            auto& textComponent = entity.GetComponent<TextComponent>();
            out << YAML::Key << "Text" << YAML::Value << textComponent.Text;
            out << YAML::Key << "Color" << YAML::Value << textComponent.Color;
            out << YAML::Key << "KerningOffset" << YAML::Value << textComponent.KerningOffset;
            out << YAML::Key << "LineSpacing" << YAML::Value << textComponent.LineSpacing;
            //out << YAML::Key << "FontAsset" << YAML::Value << textComponent.FontAsset; // TODO: Font assets

            out << YAML::EndMap;
        }

        if (entity.HasComponent<ScriptComponent>())
        {
            out << YAML::Key << "ScriptComponent";
            out << YAML::BeginMap;

            auto& script_component = entity.GetComponent<ScriptComponent>();
            out << YAML::Key << "ClassName" << YAML::Value << script_component.ClassName;

            Ref<ScriptClass> scriptClass = Script::FindEntityClass(script_component.ClassName);
            if (scriptClass)
            {
                const auto& fields = scriptClass->GetFields();
                if (fields.size() > 0)
                {
                    // Fields
                    out << YAML::Key << "Fields" << YAML::Value;
                    out << YAML::BeginSeq;

                    // Cached fields values
                    auto& cached_fields = Script::GetEntityFieldMap(uuid);

                    for (const auto& [name, field] : fields)
                    {
                        if (cached_fields.find(name) == cached_fields.end())
                            continue;

                        out << YAML::BeginMap;
                        out << YAML::Key << "Name" << YAML::Value << name;
                        out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);

                        out << YAML::Key << "Data" << YAML::Value;
                        const ScriptFieldInstance& scriptField = cached_fields.at(name);

                        switch (field.Type)
                        {
                            WRITE_SCRIPT_FIELD(Float, f32);
                            WRITE_SCRIPT_FIELD(Double, f64);
                            WRITE_SCRIPT_FIELD(Bool, bool);
                            WRITE_SCRIPT_FIELD(Char, char);
                            WRITE_SCRIPT_FIELD(Byte, i8);
                            WRITE_SCRIPT_FIELD(Short, i16);
                            WRITE_SCRIPT_FIELD(Int, i32);
                            WRITE_SCRIPT_FIELD(Long, i64);
                            WRITE_SCRIPT_FIELD(UByte, u32); // NOTE: Encoding as unsigned integer cause YAML's weird formatting for unsigned chars
                            WRITE_SCRIPT_FIELD(UShort, u16);
                            WRITE_SCRIPT_FIELD(UInt, u32);
                            WRITE_SCRIPT_FIELD(ULong, u64);
                            WRITE_SCRIPT_FIELD(Vector2, glm::vec2);
                            WRITE_SCRIPT_FIELD(Vector3, glm::vec3);
                            WRITE_SCRIPT_FIELD(Vector4, glm::vec4);
                            WRITE_SCRIPT_FIELD(Entity, UUID);
                        }

                        out << YAML::EndMap;

                    }

                    // Field
                    out << YAML::EndSeq;
                }
            }
            // ScriptComponent
            out << YAML::EndMap;
        }

        if (entity.HasComponent<AudioSourceComponent>())
        {
            out << YAML::Key << "AudioSourceComponent";
            out << YAML::BeginMap;

            auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();
            out << YAML::Key << "AudioClipPath" << YAML::Value << audioSourceComponent.Clip->GetFilepath();
            out << YAML::Key << "Gain" << YAML::Value << audioSourceComponent.Gain;
            out << YAML::Key << "Spatial" << YAML::Value << audioSourceComponent.Spatial;
            out << YAML::Key << "PlayOnStart" << YAML::Value << audioSourceComponent.PlayOnStart;
            out << YAML::Key << "Loop" << YAML::Value << audioSourceComponent.Loop;

            out << YAML::EndMap;
        }

        if (entity.HasComponent<AudioListenerComponent>())
        {
            out << YAML::Key << "AudioListenerComponent";
            out << YAML::BeginMap;

            auto& audioListenerComponent = entity.GetComponent<AudioListenerComponent>();
            out << YAML::Key << "Primary" << YAML::Value << audioListenerComponent.IsPrimary;

            out << YAML::EndMap;
        }

        if (entity.HasComponent<Rigidbody2DComponent>())
        {
            out << YAML::Key << "Rigidbody2DComponent";
            out << YAML::BeginMap;

            auto& rb2dComponent = entity.GetComponent<Rigidbody2DComponent>();
            out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2dComponent.Type);
            out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;

            out << YAML::EndMap;
        }

        if (entity.HasComponent<BoxCollider2DComponent>())
        {
            out << YAML::Key << "BoxCollider2DComponent";
            out << YAML::BeginMap;

            auto& bc2dComponent = entity.GetComponent<BoxCollider2DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
            out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
            out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
            out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.Friction;
            out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.Restitution;
            out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.RestitutionThreshold;

            out << YAML::EndMap;
        }

        if (entity.HasComponent<CircleCollider2DComponent>())
        {
            out << YAML::Key << "CircleCollider2DComponent";
            out << YAML::BeginMap;

            auto& cc2dComponent = entity.GetComponent<CircleCollider2DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << cc2dComponent.Offset;
            out << YAML::Key << "Radius" << YAML::Value << cc2dComponent.Radius;
            out << YAML::Key << "Density" << YAML::Value << cc2dComponent.Density;
            out << YAML::Key << "Friction" << YAML::Value << cc2dComponent.Friction;
            out << YAML::Key << "Restitution" << YAML::Value << cc2dComponent.Restitution;
            out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2dComponent.RestitutionThreshold;

            out << YAML::EndMap;
        }

        out << YAML::EndMap; // Entity
    }

    SceneSerializer::SceneSerializer(Ref<Scene> scene)
        : m_Scene(scene)
    {
    }

    SceneSerializer::~SceneSerializer()
    {
    }

    bool SceneSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(filepath.string());
        }
        catch (YAML::ParserException e)
        {
            return false;
        }

        if (!data["Scene"])
            return false;

        std::string sceneName = data["Scene"].as<std::string>();
        TBO_ENGINE_TRACE("Deserializing scene '{0}'", sceneName);

        auto entities = data["Entities"];
        if (entities)
        {
            for (auto entity : entities)
            {
                u64 uuid = entity["Entity"].as<u64>();

                std::string name;
                auto tagComponent = entity["TagComponent"];
                if (tagComponent)
                    name = tagComponent["Tag"].as<std::string>();

                TBO_ENGINE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

                Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

                auto transformComponent = entity["TransformComponent"];
                if (transformComponent)
                {
                    // Entities always have transforms
                    auto& tc = deserializedEntity.GetComponent<TransformComponent>();
                    tc.Translation = transformComponent["Translation"].as<glm::vec3>();
                    tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
                    tc.Scale = transformComponent["Scale"].as<glm::vec3>();
                }

                auto cameraComponent = entity["CameraComponent"];
                if (cameraComponent)
                {
                    auto& cc = deserializedEntity.AddComponent<CameraComponent>();

                    auto& cameraProps = cameraComponent["Camera"];
                    cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<i32>());

                    cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<f32>());
                    cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<f32>());
                    cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<f32>());

                    cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<f32>());
                    cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<f32>());
                    cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<f32>());

                    cc.IsPrimary = cameraComponent["Primary"].as<bool>();
                    cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
                }

                auto spriteRendererComponent = entity["SpriteRendererComponent"];
                if (spriteRendererComponent)
                {
                    auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
                    src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
                    auto& path = spriteRendererComponent["TexturePath"].as<std::string>();
                    if (path != "None")
                    {
                        Ref<Texture2D> texture2d = Texture2D::Create({ path });
                        if (texture2d->IsLoaded())
                            src.Texture = texture2d;
                        else
                            TBO_ENGINE_ERROR("Texture cannot be loaded! (\"{0}\")", path);

                    }
                }

                auto circleRendererComponent = entity["CircleRendererComponent"];
                if (circleRendererComponent)
                {
                    auto& crc = deserializedEntity.AddComponent<CircleRendererComponent>();
                    crc.Color = circleRendererComponent["Color"].as<glm::vec4>();
                    crc.Thickness = circleRendererComponent["Thickness"].as<f32>();
                    crc.Fade = circleRendererComponent["Fade"].as<f32>();
                }

                auto textComponent = entity["TextComponent"];
                if (textComponent)
                {
                    auto& tc = deserializedEntity.AddComponent<TextComponent>();
                    tc.Text = textComponent["Text"].as<std::string>();
                    //tc.FontAsset = textComponent["FontAsset"].as<std::string>(); TODO: Font Asset
                    tc.Color = textComponent["Color"].as<glm::vec4>();
                    tc.KerningOffset = textComponent["KerningOffset"].as<f32>();
                    tc.LineSpacing = textComponent["LineSpacing"].as<f32>();
                }

                auto scriptComponent = entity["ScriptComponent"];
                if (scriptComponent)
                {
                    auto& component = deserializedEntity.AddComponent<ScriptComponent>();
                    component.ClassName = scriptComponent["ClassName"].as<std::string>();

                    auto scriptFields = scriptComponent["Fields"];
                    if (scriptFields)
                    {
                        Ref<ScriptClass> entityClass = Script::FindEntityClass(component.ClassName);
                        if (entityClass)
                        {
                            const auto& fields = entityClass->GetFields();
                            auto& entityFields = Script::GetEntityFieldMap(uuid);

                            for (auto scriptField : scriptFields)
                            {
                                std::string name = scriptField["Name"].as<std::string>();
                                std::string typeString = scriptField["Type"].as<std::string>();
                                ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

                                ScriptFieldInstance& fieldInstance = entityFields[name];

                                if (fields.find(name) == fields.end())
                                    continue;

                                fieldInstance.Field = fields.at(name);

                                switch (type)
                                {
                                    READ_SCRIPT_FIELD(Float, f32);
                                    READ_SCRIPT_FIELD(Double, f64);
                                    READ_SCRIPT_FIELD(Bool, bool);
                                    READ_SCRIPT_FIELD(Char, char);
                                    READ_SCRIPT_FIELD(Byte, i8);
                                    READ_SCRIPT_FIELD(Short, i16);
                                    READ_SCRIPT_FIELD(Int, i32);
                                    READ_SCRIPT_FIELD(Long, i64);
                                    READ_SCRIPT_FIELD(UByte, u8);
                                    READ_SCRIPT_FIELD(UShort, u16);
                                    READ_SCRIPT_FIELD(UInt, u32);
                                    READ_SCRIPT_FIELD(ULong, u64);
                                    READ_SCRIPT_FIELD(Vector2, glm::vec2);
                                    READ_SCRIPT_FIELD(Vector3, glm::vec3);
                                    READ_SCRIPT_FIELD(Vector4, glm::vec4);
                                    READ_SCRIPT_FIELD(Entity, UUID);
                                }
                            }
                        }
                    }
                }

                auto audioSourceComponent = entity["AudioSourceComponent"];
                if (audioSourceComponent)
                {
                    auto& as = deserializedEntity.AddComponent<AudioSourceComponent>();

                    const auto& audioClipFilepath = audioSourceComponent["AudioClipPath"].as<std::string>();
                    as.Clip = Audio::CreateAndRegisterClip(audioClipFilepath);
                    as.Gain = audioSourceComponent["Gain"].as<f32>();
                    as.Spatial = audioSourceComponent["Spatial"].as<bool>();
                    as.PlayOnStart = audioSourceComponent["PlayOnStart"].as<bool>();
                    as.Loop = audioSourceComponent["Loop"].as<bool>();
                }

                auto audioListenerComponent = entity["AudioListenerComponent"];
                if (audioListenerComponent)
                {
                    auto& al = deserializedEntity.AddComponent<AudioListenerComponent>();
                    al.IsPrimary = audioListenerComponent["Primary"].as<bool>();
                }

                auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
                if (rigidbody2DComponent)
                {
                    auto& rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
                    rb2d.Type = RigidBody2DBodyTypeFromString(rigidbody2DComponent["BodyType"].as<std::string>());
                    rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
                }

                auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
                if (boxCollider2DComponent)
                {
                    auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();
                    bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
                    bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
                    bc2d.Density = boxCollider2DComponent["Density"].as<f32>();
                    bc2d.Friction = boxCollider2DComponent["Friction"].as<f32>();
                    bc2d.Restitution = boxCollider2DComponent["Restitution"].as<f32>();
                    bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<f32>();
                }

                auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
                if (circleCollider2DComponent)
                {
                    auto& cc2d = deserializedEntity.AddComponent<CircleCollider2DComponent>();
                    cc2d.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
                    cc2d.Radius = circleCollider2DComponent["Radius"].as<f32>();
                    cc2d.Density = circleCollider2DComponent["Density"].as<f32>();
                    cc2d.Friction = circleCollider2DComponent["Friction"].as<f32>();
                    cc2d.Restitution = circleCollider2DComponent["Restitution"].as<f32>();
                    cc2d.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<f32>();
                }
            }
        }

        return true;
    }

    bool SceneSerializer::Serialize(const std::filesystem::path& filepath)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << filepath.stem().string();
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        m_Scene->m_Registry.each([&](auto entityID)
        {
            Entity entity = { entityID, m_Scene.Get() };
            if (!entity)
                return;

            SerializeEntity(out, entity);
        });
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(filepath);

        if (fout)
        {
            fout << out.c_str();
            return true;
        }

        return false;
    }

}
