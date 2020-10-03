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

    struct AnimatedVertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec3 Binormal;
        glm::vec2 Texcoord;

        uint32_t IDs[4] = { 0, 0,0, 0 };
        float Weights[4]{ 0.0f, 0.0f, 0.0f, 0.0f };

        void AddBoneData(uint32_t BoneID, float Weight)
        {
            for (size_t i = 0; i < 4; i++)
            {
                if (Weights[i] == 0.0)
                {
                    IDs[i] = BoneID;
                    Weights[i] = Weight;
                    return;
                }
            }

            // TODO: Keep top weights
            SAT_CORE_WARN("Vertex has more than four bones/weights affecting it, extra data will be discarded (BoneID={0}, Weight={1})", BoneID, Weight);
        }
    };

    struct Index
    {
        uint32_t V1, V2, V3;
    };

    static_assert(sizeof(Index) == 3 * sizeof(uint32_t));

    struct BoneInfo
    {
        glm::mat4 BoneOffset;
        glm::mat4 FinalTransformation;
    };

    struct VertexBoneData
    {
        uint32_t IDs[4];
        float Weights[4];

        VertexBoneData()
        {
            memset(IDs, 0, sizeof(IDs));
            memset(Weights, 0, sizeof(Weights));
        };

        void AddBoneData(uint32_t BoneID, float Weight)
        {
            for (size_t i = 0; i < 4; i++)
            {
                if (Weights[i] == 0.0)
                {
                    IDs[i] = BoneID;
                    Weights[i] = Weight;
                    return;
                }
            }

            // should never get here - more bones than we have space for
            SAT_CORE_ASSERT(false, "Too many bones!");
        }
    };

    struct Triangle
    {
        Vertex V0, V1, V2;

        Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
            : V0(v0), V1(v1), V2(v2) {}
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

