#pragma once

#include <string>

#include "VertexBuffer.h"
#include "IndexBuffer.h"

struct aiNode;
struct aiScene;
struct aiMesh;

namespace Turbo
{
    // For now
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
    };

    struct Submesh
    {
        std::vector<Vertex> Vertices;
        std::vector<u32> Indices;
    };

    class StaticMesh
    {
    public:
        explicit StaticMesh(std::string_view filePath);
        ~StaticMesh();

        u32 GetIndicesPerInstance() const { return (u32)m_Indices.size(); }

        Ref<VertexBuffer> GetVertexBuffer() const { return m_VertexBuffer; }
        Ref<IndexBuffer> GetIndexBuffer() const { return m_IndexBuffer; }

        const std::vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }
    private:
        void Load();
        void AllocateGPUResources();
        void ProcessNode(const aiScene* scene, aiNode* node);
        Submesh ProcessMesh(const aiScene* scene, aiMesh* mesh);
    private:
        std::vector<Submesh> m_Submeshes;

        u32 m_TotalVerticesCount = 0;
        u32 m_TotalIndicesCount = 0;
        std::vector<Vertex> m_Vertices;
        std::vector<u32> m_Indices;

        Ref<VertexBuffer> m_VertexBuffer;
        Ref<IndexBuffer> m_IndexBuffer;

        std::string_view m_FilePath;
    };
}
