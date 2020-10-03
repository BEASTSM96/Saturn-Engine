#include "sppch.h"
#include "Model.h"

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>


namespace Saturn {

	Model::Model(std::string const& path, bool gamma)
        : gammaCorrection(gamma)
    {
        transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, 0.0f));
        transform = glm::scale(transform, glm::vec3(1.0f, 1.0f, 1.0f));

        transform = transform;

        m_Shader = new DShader("assets/shaders/3d_test.satshaderv", "assets/shaders/3d_test.satshaderf");

        m_Directory = path.substr(0, path.find_last_of('/'));
        m_Directory = m_Directory.substr(0, path.find_last_of('\\'));


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
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices
        );


        if (!s || s->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !s->mRootNode)
        {
            SAT_CORE_ERROR("Failed to load model: {0} ", m_Importer.GetErrorString());
            return;
        }

        directory = path.substr(0, path.find_last_of('/'));

        processNode(s->mRootNode, s);
        ProcessMaterials(s);
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
        }

    }



    std::vector<FTexture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
    {
        std::vector<FTexture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip)
            {   // if texture hasn't been loaded already, load it
                FTexture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }

    unsigned int Model::TextureFromFile(const char* path, const std::string& directory, bool gamma)
    {
        std::string filename = std::string(path);
        filename = directory + '/' + filename;

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
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }
}