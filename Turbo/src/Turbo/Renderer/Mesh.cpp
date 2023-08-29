#include "tbopch.h"
#include "Mesh.h"

#include "Turbo/Debug/ScopeTimer.h"
#include "Turbo/Renderer/ShaderLibrary.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

namespace Turbo {

    namespace Utils {

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

    struct DebugLogStream : public Assimp::LogStream
    {
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


    struct AssimpLoader
    {
        Assimp::Importer Importer;

        AssimpLoader()
        {
            Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
            Assimp::DefaultLogger::get()->attachStream(new DebugLogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
        }

        ~AssimpLoader()
        {
            Assimp::DefaultLogger::kill();
        }

        const aiScene* ReadFile(std::string_view filepath)
        {
            const aiScene* scene = Importer.ReadFile(filepath.data(), s_AssimpImporterFlags);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            {
                TBO_ENGINE_ERROR("Could not load static mesh! {}", Importer.GetErrorString());
                return nullptr;
            }

            return scene;
        }

        const aiScene* ReadFileFromMemory(Buffer buffer)
        {
            const aiScene* scene = Importer.ReadFileFromMemory(buffer.Data, buffer.Size, s_AssimpImporterFlags, "fbx");

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            {
                TBO_ENGINE_ERROR("Could not load static mesh! {}", Importer.GetErrorString());
                return nullptr;
            }

            return scene;

        }
    };

    // Relying on static initialization/destruction
    static AssimpLoader s_AssimpLoader;

    // ###############################################################################################################
    // ##################                                MeshSource                                 ##################
    // ###############################################################################################################

    MeshSource::MeshSource(std::string_view filePath)
        : m_FilePath(filePath)
    {
        Load(s_AssimpLoader.ReadFile(m_FilePath));
    }

    MeshSource::MeshSource(Buffer buffer)
    {
        Load(s_AssimpLoader.ReadFileFromMemory(buffer));
    }

    void MeshSource::Load(const aiScene* scene)
    {
        if (!scene)
            return;

        // Get static mesh shader
        m_MeshShader = ShaderLibrary::Get("StaticMesh");

        // Process and flatten the hierarchy of the mesh
        ProcessNode(scene, scene->mRootNode);

        // Allocate GPU resources
        m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex));
        m_IndexBuffer = IndexBuffer::Create(m_Indices);

        m_Loaded = true;
    }

    // Flattens the hierarchy 
    void MeshSource::ProcessNode(const aiScene* scene, aiNode* node)
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
            submesh.IndexCount = mesh->mNumFaces * 3; // If the geometry is not made out of triangles we're gonna have a big problem
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

    void MeshSource::TraverseNodes(aiNode* node, glm::mat4 parentTransform, u32 level)
    {
        glm::mat4 localTransform = Utils::Mat4FromAssimpMat4(node->mTransformation);
        glm::mat4 transform = parentTransform * localTransform;

        for (u32 i = 0; i < node->mNumMeshes; i++)
        {
            u32 mesh = node->mMeshes[i];
            auto& submesh = m_Submeshes[mesh];
            submesh.Transform = transform;
#ifdef TBO_DEBUG 
            submesh.DebugName = node->mName.C_Str();
#endif
        }

        for (u32 i = 0; i < node->mNumChildren; i++)
            TraverseNodes(node->mChildren[i], transform, level + 1);
    }

    // ###############################################################################################################
    // ##################                               Static Mesh                                 ##################
    // ###############################################################################################################

    StaticMesh::StaticMesh(const Ref<MeshSource>& meshSource, const std::vector<u32>& submeshIndices)
        : m_MeshSource(meshSource), m_SubmeshIndices(submeshIndices)
    {
    }

    StaticMesh::StaticMesh(const Ref<MeshSource>& meshSource)
        : m_MeshSource(meshSource)
    {
        // Default setting
        auto& submeshes = meshSource->GetSubmeshes();
        m_SubmeshIndices.resize(submeshes.size());
        for (u32 i = 0; i < m_SubmeshIndices.size(); i++)
        {
            m_SubmeshIndices[i] = i;
        }
    }

}
