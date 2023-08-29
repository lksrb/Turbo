#pragma once

#include "VertexBuffer.h"
#include "Shader.h"
#include "IndexBuffer.h"

#include "Turbo/Asset/Asset.h"
#include "Turbo/Core/Buffer.h"

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

#ifdef TBO_DEBUG
        std::string DebugName;
#endif
    };

    class MeshSource : public Asset
    {
    public:
        MeshSource(Buffer buffer);
        MeshSource(std::string_view filePath);
        ~MeshSource() = default;

        Ref<VertexBuffer> GetVertexBuffer() const { return m_VertexBuffer; }
        Ref<IndexBuffer> GetIndexBuffer() const { return m_IndexBuffer; }
        Ref<Shader> GetMeshShader() const { return m_MeshShader; }

        const std::vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }

        bool IsLoaded() const { return m_Loaded; }
        AssetType GetAssetType() const override { return AssetType_MeshSource; }
        static constexpr AssetType GetStaticAssetType() { return AssetType_MeshSource; }
    private:
        void TraverseNodes(aiNode* node, glm::mat4 parentTransform = glm::mat4(1.0), u32 level = 0);
        void Load(const aiScene* scene);
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

        Ref<Shader> m_MeshShader;
        Ref<VertexBuffer> m_VertexBuffer;
        Ref<IndexBuffer> m_IndexBuffer;

        std::string m_FilePath;
    };

    class StaticMesh : public Asset
    {
    public:
        StaticMesh(const Ref<MeshSource>& meshSource);
        StaticMesh(const Ref<MeshSource>& meshSource, const std::vector<u32>& submeshIndices);
        ~StaticMesh() = default;

        const std::vector<u32>& GetSubmeshIndices() const { return m_SubmeshIndices; }
        const Ref<MeshSource>& GetMeshSource() const { return m_MeshSource; }

        AssetType GetAssetType() const override { return AssetType_StaticMesh; }
        static constexpr AssetType GetStaticAssetType() { return AssetType_StaticMesh; }
    private:
        Ref<MeshSource> m_MeshSource;
        std::vector<u32> m_SubmeshIndices;

        // TODO: Material
    };
}
