#include "sppch.h"
#include "Model.h"

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include "Saturn/Renderer/Material.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>


namespace Saturn {

    Model::Model(std::string const& path, std::string ShaderVertexPath, std::string ShaderFragmentPath, bool gamma) : gammaCorrection(gamma)
    {
        m_Shader = new DShader(ShaderVertexPath.c_str(), ShaderFragmentPath.c_str());
    

        m_Directory = path.substr(0, path.find_last_of('/'));
        m_Directory = m_Directory.substr(0, path.find_last_of('\\'));

        m_Name = "Emtpy Model";

        loadModel(path);

    }

    void Model::Draw(DShader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

    void Model::Update(Timestep ts, DShader& shader)
    {
    }

    void Model::loadModel(std::string const& path)
    {
        Assimp::Importer m_Importer;

        const aiScene* s = m_Importer.ReadFile(
            path,
            aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
            aiProcess_Triangulate |             // Make sure we're triangles
            aiProcess_SortByPType |             // Split meshes by primitive type
            aiProcess_GenNormals |              // Make sure we have legit normals
            aiProcess_GenUVCoords |             // Convert UVs if required 
            aiProcess_OptimizeMeshes |          // Batch draws where possible
            aiProcess_ValidateDataStructure   // Validation
        );


        if (!s || s->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !s->mRootNode)
        {
            SAT_CORE_ERROR("Failed to load model: {0} ", m_Importer.GetErrorString());
            return;
        }

        directory = path.substr(0, path.find_last_of('/'));

        m_IsAnimated = s->mAnimations != nullptr;


        processNode(s->mRootNode, s);

        if (s->HasAnimations())
        {
            delete m_Shader;
            m_Shader = new DShader("assets/shaders/3D_TestAnimation.glsl", "assets/shaders/3D_TestAnimationv.glsl");
            for (size_t m = 0; m < s->mNumMeshes; m++)
            {
                aiMesh* mesh = s->mMeshes[m];
                ProcessAnimations(mesh, s);
            }

        }

        /* Might not need to do this*/
        if (s->HasMaterials())
        {
            ProcessMaterials(s);
        }


    }

    void Model::processNode(aiNode* node, const aiScene* scene)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<FTexture> textures;


        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {

            SAT_CORE_ASSERT(mesh->HasPositions(), "Meshes require positions.");
            SAT_CORE_ASSERT(mesh->HasNormals(), "Meshes require normals.");

            Vertex vertex;
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);


            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back((unsigned short)face.mIndices[j]);
            }
        }



        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }

    void Model::ProcessMaterials(const aiScene* scene)
    {
        for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
            aiMaterial* material = scene->mMaterials[i];

            std::string diffusePath;
            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                aiString str;
                material->GetTexture(aiTextureType_DIFFUSE, 0, &str);
                diffusePath = m_Directory + "/" + str.C_Str();
            }

            std::string roughnessPath;
            if (material->GetTextureCount(aiTextureType_UNKNOWN) > 0) {
                aiString str;
                material->GetTexture(aiTextureType_UNKNOWN, 0, &str);
                roughnessPath = m_Directory + "/" + str.C_Str();
            }

            std::string normalPath;
            if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
                aiString str;
                material->GetTexture(aiTextureType_NORMALS, 0, &str);
                normalPath = m_Directory + "/" + str.C_Str();
            }

            aiColor3D color(0.8, 0.8, 0.8);
            material->Get(AI_MATKEY_COLOR_DIFFUSE, color);

            const char* diffuseTex = !diffusePath.empty() ? diffusePath.c_str() : nullptr;
            const char* roughnessTex = !roughnessPath.empty() ? roughnessPath.c_str() : nullptr;
            const char* normalTex = !normalPath.empty() ? normalPath.c_str() : nullptr;

            const char* materialName = material->GetName().length != 0 ? material->GetName().C_Str() : "Unnamed Material";

            if (diffusePath.empty())
            {
                diffusePath = "assets/tex/default/Checkerboard.tga";
            }

            /*Should not really use a raw pointer here!*/
            Material* mat = Material::Create(
                materialName,
                glm::vec3(color.r, color.g, color.b),
                glm::vec3(color.r, color.g, color.b),
                glm::vec3(color.r, color.g, color.b),
                TextureFromFile(diffusePath.c_str(), diffusePath),
                TextureFromFile(diffusePath.c_str(), diffusePath)
            );

            mat->SendToShader(m_Shader);

            m_Materials.push_back(mat);

        }

    }

    void Model::ProcessAnimations(aiMesh* mesh, const aiScene* scene)
    {
        // Vertices
        if (m_IsAnimated)
        {
            for (size_t i = 0; i < mesh->mNumVertices; i++)
            {
                AnimatedVertex vertex;
                vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
                vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

                if (mesh->HasTangentsAndBitangents())
                {
                    vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
                    vertex.Binormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
                }

                if (mesh->HasTextureCoords(0))
                    vertex.Texcoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

                m_AnimatedVertices.push_back(vertex);
            }
        }

    }


    unsigned int Model::TextureFromFile(const char* path, const std::string& directory, bool gamma)
    {
        std::string filename = std::string(path);
        std::string filepath = std::string(path);
        filename = filepath;

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << " directory " << directory << " filename " << filename << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }
}