#include "Mesh.h"
#include "Saturn/Renderer/3D/3dShader.h"
#include "Saturn/Scene/Scene.h"


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>

namespace Saturn {

    class SATURN_API Model
    {
    public:
        // constructor, expects a filepath to a 3D model.
        Model(std::string const& path, bool gamma = false);

        // draws the model, and thus all its meshes
        void Draw(DShader& shader);

        void Update(Timestep ts, DShader& shader);

    public:
                // model data 
        std::vector<FTexture>   textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
        std::vector<Mesh>       meshes;
        std::string             directory;
        bool                    gammaCorrection;

        DShader * GetShader() {
            return new DShader("assets/shaders/3d_test.satshaderv", "assets/shaders/3d_test.satshaderf");
        }

       const glm::mat4& GetTransform() {
            return transform;
       }


        const glm::mat4& SetTransform(glm::mat4 & newtransform) {
            return transform = newtransform;
        }

    private:
        void ProcessMaterials(const aiScene* scene);

    private:
        // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
        void loadModel(std::string const& path);

        // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
        void processNode(aiNode* node, const aiScene* scene);

        Mesh processMesh(aiMesh* mesh, const aiScene* scene);

        std::vector<FTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

        unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

       glm::mat4                oldtransform = glm::mat4(0.0f);

       glm::mat4               transform = glm::mat4(1.0f);

       DShader      *           m_Shader;

       std::string m_Path;
       std::string m_Directory;

    };
}