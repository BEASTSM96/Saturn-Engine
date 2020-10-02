#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

namespace Saturn {

    class SATURN_API DShader
    {
    public:
        unsigned int ID;
        // constructor generates the shader

        DShader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
        ~DShader();

        // activate the shader & deactivate the shader

        void Bind();

        void Unbind();

        // utility uniform functions

        void UploadBool(const std::string& name, bool value) const;


        void UploadInt(const std::string& name, int value) const;


        void UploadFloat(const std::string& name, float value) const;


        void UploadFloat2(const std::string& name, const glm::vec2& value) const;

        void UploadFloat2(const std::string& name, float x, float y) const;


        void UploadFloat3(const std::string& name, const glm::vec3& value) const;

        void UploadFloat3(const std::string& name, float x, float y, float z) const;


        void UploadFloat4(const std::string& name, const glm::vec4& value) const;

        void UploadFloat4(const std::string& name, float x, float y, float z, float w);


        void UploadMat2(const std::string& name, const glm::mat2& mat) const;


        void UploadMat3(const std::string& name, const glm::mat3& mat) const;


        void UploadMat4(const std::string& name, const glm::mat4& mat) const;

        void UploadTextureType(const std::string& name, unsigned int value) const;

        ////////////////////////////////////////////////////////////////////////////////////

        int GetInt(const std::string& name) const;


    private:
        void dispatch(const unsigned int sizeX, const unsigned int sizeY, const unsigned int sizeZ) const;


        // utility function for checking shader compilation/linking errors.
        // ------------------------------------------------------------------------
        void checkCompileErrors(GLuint shader, std::string type);
    };

}

#endif