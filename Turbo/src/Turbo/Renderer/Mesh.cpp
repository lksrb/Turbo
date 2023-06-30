#include "tbopch.h"
#include "Mesh.h"

#include "Turbo/Debug/ScopeTimer.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

namespace Turbo
{
    struct LogDebugStream : public Assimp::LogStream
    {
        static Assimp::Importer Importer;

        static void Initialize()
        {
            if (Assimp::DefaultLogger::isNullLogger())
            {
                Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
                Assimp::DefaultLogger::get()->attachStream(new LogDebugStream, Assimp::Logger::Err | Assimp::Logger::Warn);
            }
        }

        virtual void write(const char* message) override
        {
            TBO_ENGINE_ERROR("Assimp: {}", message);
        }
    };


    StaticMesh::StaticMesh(std::string_view filePath)
        : m_FilePath(filePath)
    {
        LogDebugStream::Initialize();

        Load();
        AllocateGPUResources();
    }

    StaticMesh::~StaticMesh()
    {

    }

    void StaticMesh::Load()
    {
        Assimp::Importer importer;

        // aiProcess_FlipUVs - Flip UVs but I think vulkan already has fliped UVs
        const aiScene* scene = importer.ReadFile(m_FilePath.data(), aiProcess_Triangulate | aiProcess_GenNormals);
        TBO_ENGINE_ASSERT(scene && ~scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE && scene->mRootNode, importer.GetErrorString());

        ProcessNode(scene, scene->mRootNode);
    }

    void StaticMesh::AllocateGPUResources()
    {
        m_Vertices.reserve(m_TotalVerticesCount);
        m_Indices.reserve(m_TotalIndicesCount);

        for (auto& submesh : m_Submeshes)
        {
            for (auto& vertex : submesh.Vertices)
            {
                m_Vertices.push_back(vertex);
            }
            for (auto& index : submesh.Indices)
            {
                m_Indices.push_back(index);
            }
        }

        m_VertexBuffer = VertexBuffer::Create({ m_Vertices.size() * sizeof(Vertex) });
        m_VertexBuffer->SetData(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex));

        m_IndexBuffer = IndexBuffer::Create(m_Indices);
    }

    void StaticMesh::ProcessNode(const aiScene* scene, aiNode* node)
    {
        for (u32 i = 0; i < node->mNumMeshes; i++)
        {
            u32 meshIndex = node->mMeshes[i];
            aiMesh* mesh = scene->mMeshes[meshIndex];
            m_Submeshes.push_back(ProcessMesh(scene, mesh));
        }

        for (u32 i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(scene, node->mChildren[i]);
        }
    }

    Submesh StaticMesh::ProcessMesh(const aiScene* scene, aiMesh* mesh)
    {
        std::vector<Vertex> vertices;
        std::vector<u32> indices;

        vertices.reserve(mesh->mNumVertices);
        for (u32 i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex& vertex = vertices.emplace_back();
            vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
            vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
            vertex.TexCoords = glm::vec2(0.0f);
            // Does the mesh contain texture coords
            if (mesh->HasTextureCoords(0))
            {
                vertex.TexCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
            }

            m_TotalVerticesCount++;
        }


        // Process indices
        // NOTE: Faces are equal to number of primitives
        // Preallocation is roughly based on number of vertices
        indices.reserve(mesh->mNumFaces * 3);
        for (u32 i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];

            for (u32 j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
                m_TotalIndicesCount++;
            }
        }

        return { vertices, indices };
    }

}
