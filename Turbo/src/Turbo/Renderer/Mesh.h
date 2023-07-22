#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include "Turbo/Asset/Asset.h"

#include <string>

struct aiNode;
struct aiScene;
struct aiMesh;

namespace Turbo
{
    struct Submesh
    {
        u32 BaseVertex = 0;
        u32 BaseIndex = 0;
        u32 VertexCount = 0;
        u32 IndexCount = 0;
        glm::mat4 Transform{ 1.0f };
        std::string Name;
    };

    class StaticMesh : public Asset
    {
    public:
        StaticMesh(std::string_view filePath);
        ~StaticMesh();

        static Ref<StaticMesh> Create(std::string_view filePath);

        Ref<VertexBuffer> GetVertexBuffer() const { return m_VertexBuffer; }
        Ref<IndexBuffer> GetIndexBuffer() const { return m_IndexBuffer; }

        const std::vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }

        bool IsLoaded() const { return m_Loaded; }
        AssetType GetAssetType() const override { return AssetType_StaticMesh; }
    private:
        void TraverseNodes(aiNode* node, glm::mat4 parentTransform = glm::mat4(1.0), u32 level = 0);
        void Load();
        void ProcessNode(const aiScene* scene, aiNode* node);
    private:
        struct Vertex
        {
            glm::vec3 Position;
            glm::vec3 Normal;
            glm::vec2 TexCoords;
        };

        std::vector<Submesh> m_Submeshes;

        bool m_Loaded = false;
        u32 m_TotalVerticesCount = 0;
        u32 m_TotalIndicesCount = 0;
        std::vector<Vertex> m_Vertices;
        std::vector<u32> m_Indices;

        Ref<VertexBuffer> m_VertexBuffer;
        Ref<IndexBuffer> m_IndexBuffer;

        std::string m_FilePath;
    };
}
