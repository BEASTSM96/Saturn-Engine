#include "sppch.h"
#include "OpenGLMaterial.h"

#include "Saturn/Renderer/3D/3dShader.h"

namespace Saturn {

	OpenGLMaterial::OpenGLMaterial(std::string Name, glm::vec3 Ambient, glm::vec3 Diffuse, glm::vec3 Specular, GLuint DiffuseTexture, GLuint SpecularTexture)
	{
		this->Ambient = Ambient;
		this->Diffuse = Diffuse;
		this->Specular = Specular;
		this->DiffuseTexture = DiffuseTexture;
		this->DiffuseTexture = SpecularTexture;

		this->MaterialName = Name;
	}

	OpenGLMaterial::~OpenGLMaterial()
	{
	}

	void OpenGLMaterial::Bind()
	{
	}

	void OpenGLMaterial::Unbind()
	{
	}

	void OpenGLMaterial::SendToShader(DShader Shader)
	{
		Shader.UploadFloat3("material.ambient", this->Ambient);
		Shader.UploadFloat3("material.diffuse", this->Diffuse);
		Shader.UploadFloat3("material.specular", this->Specular);
		Shader.UploadInt("material.diffuseTex", this->DiffuseTexture);
		Shader.UploadInt("material.specularTex", this->SpecularTexture);
	}

	void OpenGLMaterial::SendToShader(DShader* Shader)
	{
		Shader->UploadFloat3("material.ambient", this->Ambient);
		Shader->UploadFloat3("material.diffuse", this->Diffuse);
		Shader->UploadFloat3("material.specular", this->Specular);
		Shader->UploadInt("material.diffuseTex", this->DiffuseTexture);
		Shader->UploadInt("material.specularTex", this->SpecularTexture);
	}

}