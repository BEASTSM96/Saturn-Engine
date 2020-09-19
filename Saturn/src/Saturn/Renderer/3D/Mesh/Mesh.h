#include "Saturn/Renderer/3D/3dShader.h"

#include "Saturn/Renderer/VertexArray.h"

#include <string>
#include <vector>

#ifndef MODEL
#define MODEL


namespace Saturn {

    struct Vertex {
        // position
        glm::vec3 Position;
        // normal
        glm::vec3 Normal;
        // texCoords
        glm::vec2 TexCoords;
        // tangent
        glm::vec3 Tangent;
        // bitangent
        glm::vec3 Bitangent;
    };

    struct FTexture {
        unsigned int id;
        std::string type;
        std::string path;
    };


    class SATURN_API Mesh 
    {
    public:

        // constructor
        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<FTexture> textures);

        // render the mesh
        void Draw(DShader& shader);
       

    public:
        std::vector<Vertex>         vertices;
        std::vector<unsigned int>   indices;
        std::vector<FTexture>       textures;
        unsigned int VAO;

    private:

        unsigned int VBO, EBO;

        void setupMesh();
    };

}
#endif // !MODEL

