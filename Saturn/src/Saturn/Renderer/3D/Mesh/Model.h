#include "Mesh.h"
#include "Saturn/Renderer/3D/3dShader.h"
#include "Saturn/Scene/Scene.h"

#include "Saturn/Renderer/Material.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>

namespace Saturn {

    class SATURN_API Model
    {
    public:

        Model(std::string const& path, std::string ShaderVertexPath, std::string ShaderFragmentPath, bool gamma = false);

        void Draw(DShader& shader);

        void Update(Timestep ts, DShader& shader);

    public:

        std::vector<FTexture>   textures_loaded;
        std::vector<Mesh>       meshes;
        std::string             directory;
        bool                    gammaCorrection;

        DShader* GetShader() {
            return m_Shader;
        }

        std::vector<Material*> GetMaterial() {
            return m_Materials;
        }

        std::string& GetName() {
            return m_Name;
        }
    private:

        void ProcessMaterials(const aiScene* scene);


        void loadModel(std::string const& path);

        void processNode(aiNode* node, const aiScene* scene);

        Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    private:
        std::vector<FTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
        unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

        std::vector<Material*> m_Materials;

        DShader* m_Shader;

        std::string m_Path;
        std::string m_Directory;
        std::string m_Name;

    private:
        friend class GameObject;
    };
}