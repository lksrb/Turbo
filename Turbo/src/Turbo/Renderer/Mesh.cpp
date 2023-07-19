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
    namespace Utils
    {
        glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4& matrix)
        {
            glm::mat4 result;
            //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
            result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
            result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
            result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
            result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
            return result;
        }
    }

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


    static u32 s_AssimpImporterFlags =
        aiProcess_Triangulate // Ensures that everything is a triangle
        | aiProcess_GenNormals // Generate normals if needed
        | aiProcess_OptimizeMeshes // Reduces the number of meshes if possible
        | aiProcess_GlobalScale // For .fbx import
        | aiProcess_JoinIdenticalVertices // Optimizes the mesh
        | aiProcess_ValidateDataStructure;


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
        static Ref<Assimp::Importer> s_Importer;
        
        if (!s_Importer)
        {
            s_Importer = Ref<Assimp::Importer>::Create();
        }

        // aiProcess_FlipUVs - Flip UVs but I think vulkan already has fliped UVs
        const aiScene* scene = s_Importer->ReadFile(m_FilePath.data(), s_AssimpImporterFlags);
        TBO_ENGINE_ASSERT(scene && ~scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE && scene->mRootNode, s_Importer->GetErrorString());

        ProcessNode(scene, scene->mRootNode);
    }

    void StaticMesh::AllocateGPUResources()
    {
        m_VertexBuffer = VertexBuffer::Create({ m_Vertices.size() * sizeof(Vertex) });
        m_VertexBuffer->SetData(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex));

        m_IndexBuffer = IndexBuffer::Create(m_Indices);
    }

    // Flattens the hierarchy 
    void StaticMesh::ProcessNode(const aiScene* scene, aiNode* node)
    {
        u32 vertexCount = 0;
        u32 indexCount = 0;

        m_Submeshes.reserve(scene->mNumMeshes);
        for (u32 i = 0; i < scene->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[i];
            auto& submesh = m_Submeshes.emplace_back();
            submesh.BaseVertex = vertexCount;
            submesh.BaseIndex = indexCount;
            submesh.VertexCount = mesh->mNumVertices;
            submesh.IndexCount = mesh->mNumFaces * 3; // It the geometry is not triangle we're gonna have a big problem
            vertexCount += mesh->mNumVertices;
            indexCount += submesh.IndexCount;

            // Vertices
            for (u32 i = 0; i < mesh->mNumVertices; i++)
            {
                Vertex& vertex = m_Vertices.emplace_back();
                vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
                vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
                vertex.TexCoords = glm::vec2(0.0f);
                // Does the mesh contain texture coords
                if (mesh->HasTextureCoords(0))
                {
                    vertex.TexCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
                }
            }


            // Indices
            for (u32 i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];

                for (u32 j = 0; j < face.mNumIndices; j++)
                {
                    m_Indices.push_back(face.mIndices[j]);
                }
            }

        }

        TraverseNodes(scene->mRootNode);
    }

    void StaticMesh::TraverseNodes(aiNode* node, glm::mat4 parentTransform, u32 level)
    {
        glm::mat4 localTransform = Utils::Mat4FromAssimpMat4(node->mTransformation);
        glm::mat4 transform = parentTransform * localTransform;

        for (u32 i = 0; i < node->mNumMeshes; i++)
        {
            u32 mesh = node->mMeshes[i];
            auto& submesh = m_Submeshes[mesh];
            submesh.Name = node->mName.C_Str();
            submesh.Transform = transform;
        }

        for (u32 i = 0; i < node->mNumChildren; i++)
            TraverseNodes(node->mChildren[i], transform, level + 1);
    }
}
