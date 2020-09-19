#include "sppch.h"
#include "GameObject.h"
#include "Saturn/Core/Serialisation/Object.h"

#include "GameLayer.h"

#include "Saturn/Application.h"
#include "Saturn/Scene/Components.h"

#include "Platform/OpenGL/OpenGLShader.h"


#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/type_ptr.hpp>



#ifdef SPARKY_GAME_BASE

namespace Saturn {

	GameObject* GameObject::s_Instance = nullptr;

	GameObject::GameObject() : Serialiser::OBJ_NAME("GameObject")
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//AddComponent<TransformComponent>();																			   /

		//auto& tag = AddComponent<TagComponent>();
		//tag.Tag = "Empty GameObject";																					    /
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	}

	GameObject::GameObject(const std::string& objectname, Json::Value& reconstructionValue) : Serialiser::OBJ_NAME("GameObject")
	{

		m_SquareVA.reset(Saturn::VertexArray::Create());

		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
		};

		Saturn::Ref<Saturn::VertexBuffer> squareVB;
		squareVB.reset(Saturn::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ Saturn::ShaderDataType::Float3, "a_Position" },
			{ Saturn::ShaderDataType::Float2, "a_TexCoord" }
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Saturn::Ref<Saturn::IndexBuffer> squareIB;
		squareIB.reset(Saturn::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);


		std::string textureShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec2 a_TexCoord;
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			out vec2 v_TexCoord;
			void main()
			{
				v_TexCoord = a_TexCoord;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
			}
		)";

		std::string textureShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;
			in vec2 v_TexCoord;
			
			uniform sampler2D u_Texture;
			void main()
			{
				color = texture(u_Texture, v_TexCoord);
			}
		)";

		m_playerShader.reset(Saturn::Shader::Create(textureShaderVertexSrc, textureShaderFragmentSrc));


		m_playerTexture = Saturn::Texture2D::Create("assets/tex/beastpic_pfp_1.png");

		std::dynamic_pointer_cast<Saturn::OpenGLShader>(m_playerShader)->Bind();
		std::dynamic_pointer_cast<Saturn::OpenGLShader>(m_playerShader)->UploadUniformInt("u_Texture", 0);
	}

	GameObject::GameObject(entt::entity handle, Scene* scene) : Serialiser::OBJ_NAME("GameObject"), m_EntityHandle(handle), m_Scene(scene)
	{

	}

	void GameObject::Init() {

		s_Instance = this;

		// tell stb_image.h to flip loaded texture's on the y-axis
		stbi_set_flip_vertically_on_load(true);


		GetComponent<TransformComponent>().Transform = glm::translate(transform, glm::vec3(10.0f, 10.0f, 10.0f));
		GetComponent<TransformComponent>().Transform = glm::scale(transform, glm::vec3(1.0f, 1.0f, 1.0f));

		if (m_3D)
		{																							//Will prob be GameObject::s_Instance->GetMeshComp()
			ourShader = new DShader("assets/shaders/3d_test.vs", "assets/shaders/3d_test.fs");

			ourModel = new Model("assets/meshes/base_cone.obj");

	
			shaders.push_back(ourShader);
		}

		GameObjectState = E_GameObjectState::Idle;

		GameLayer* gl = Application::Get().m_gameLayer;

		gl->gameObjects.push_back(*this);

		SAT_CORE_WARN("GameObject Size {0} " , gl->gameObjects.size());

	}

	void GameObject::OnKeyInput(KeyPressedEvent& InEvent)
	{
	}

	void GameObject::OnPos()
	{
		m_PlayerPosition.x += 50.0f * 0.01f;
	}

	void GameObject::OnUpdate(Timestep ts) {
	
		ourModel->SetTransform(GetComponent<TransformComponent>().Transform);
		ourModel->Update(ts, *ourShader);
	
	}

	void GameObject::Render()
	{	
		if (!m_3D)
		{
			GameLayer* gl = Application::Get().m_gameLayer;

			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.1f));

			FTransform tras = FTransform(m_PlayerPosition, scale, 0.0f);

			gl->Sumbit(m_playerShader, m_SquareVA, tras);
			//m_playerTexture->Bind();
		}
		else 
		{
			GameLayer* gl = Application::Get().m_gameLayer;

			ourShader->use();

			// view/projection transformations
			glm::mat4 projection = glm::perspective(glm::radians(Application::Get().m_gameLayer->m_3DCamera.Zoom), (float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight(), 0.1f, 100.0f);
			glm::mat4 view = Application::Get().m_gameLayer -> m_3DCamera.GetViewMatrix();
			ourShader -> setMat4("projection", projection);
			ourShader -> setMat4("view", view);

			ourShader->setMat4("model", GetComponent<TransformComponent>().Transform);

	
			ourModel -> Draw(*ourShader);

		}
	}



	// renders (and builds at first invocation) a sphere
	// -------------------------------------------------
	unsigned int sphereVAO = 0;
	unsigned int indexCount;
	void GameObject::renderSphere()
	{
		if (sphereVAO == 0)
		{
			glGenVertexArrays(1, &sphereVAO);

			unsigned int vbo, ebo;
			glGenBuffers(1, &vbo);
			glGenBuffers(1, &ebo);

			std::vector<glm::vec3> positions;
			std::vector<glm::vec2> uv;
			std::vector<glm::vec3> normals;
			std::vector<unsigned int> indices;

			const unsigned int X_SEGMENTS = 64;
			const unsigned int Y_SEGMENTS = 64;
			const float PI = 3.14159265359;
			for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
			{
				for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
				{
					float xSegment = (float)x / (float)X_SEGMENTS;
					float ySegment = (float)y / (float)Y_SEGMENTS;
					float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
					float yPos = std::cos(ySegment * PI);
					float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

					positions.push_back(glm::vec3(xPos, yPos, zPos));
					uv.push_back(glm::vec2(xSegment, ySegment));
					normals.push_back(glm::vec3(xPos, yPos, zPos));
				}
			}

			bool oddRow = false;
			for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
			{
				if (!oddRow) // even rows: y == 0, y == 2; and so on
				{
					for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
					{
						indices.push_back(y * (X_SEGMENTS + 1) + x);
						indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					}
				}
				else
				{
					for (int x = X_SEGMENTS; x >= 0; --x)
					{
						indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
						indices.push_back(y * (X_SEGMENTS + 1) + x);
					}
				}
				oddRow = !oddRow;
			}
			indexCount = indices.size();

			std::vector<float> data;
			for (unsigned int i = 0; i < positions.size(); ++i)
			{
				data.push_back(positions[i].x);
				data.push_back(positions[i].y);
				data.push_back(positions[i].z);
				if (uv.size() > 0)
				{
					data.push_back(uv[i].x);
					data.push_back(uv[i].y);
				}
				if (normals.size() > 0)
				{
					data.push_back(normals[i].x);
					data.push_back(normals[i].y);
					data.push_back(normals[i].z);
				}
			}
			glBindVertexArray(sphereVAO);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
			float stride = (3 + 2 + 3) * sizeof(float);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
		}

		glBindVertexArray(sphereVAO);
		glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
	}

	void GameObject::OnPBRRender()
	{
#if 0



		shader->use();
		shader->setVec3("albedo", 0.5f, 0.0f, 0.0f);
		shader->setFloat("ao", 1.0f);

		// lights
		// ------
		glm::vec3 lightPositions[] = {
			glm::vec3(-10.0f,  10.0f, 10.0f),
			glm::vec3(10.0f,  10.0f, 10.0f),
			glm::vec3(-10.0f, -10.0f, 10.0f),
			glm::vec3(10.0f, -10.0f, 10.0f),
		};
		glm::vec3 lightColors[] = {
			glm::vec3(300.0f, 300.0f, 300.0f),
			glm::vec3(300.0f, 300.0f, 300.0f),
			glm::vec3(300.0f, 300.0f, 300.0f),
			glm::vec3(300.0f, 300.0f, 300.0f)
		};
		int nrRows = 7;
		int nrColumns = 7;
		float spacing = 2.5;

		// initialize static shader uniforms before rendering
		// --------------------------------------------------
		glm::mat4 projection2 = glm::perspective(glm::radians(Application::Get().m_gameLayer->m_3DCamera.Zoom), (float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight(), 0.1f, 100.0f);
		shader->use();
		shader->setMat4("projection", projection2);

		shader->use();
		glm::mat4 view2 = Application::Get().m_gameLayer->m_3DCamera.GetViewMatrix();
		shader->setMat4("view", view2);
		shader->setVec3("camPos", Application::Get().m_gameLayer->m_3DCamera.Position);

		// render rows*column number of spheres with varying metallic/roughness values scaled by rows and columns respectively
		glm::mat4 model2 = glm::mat4(1.0f);
		for (int row = 0; row < nrRows; ++row)
		{
			shader->setFloat("metallic", (float)row / (float)nrRows);
			for (int col = 0; col < nrColumns; ++col)
			{
				// we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
				// on direct lighting.
				shader->setFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));

				model2 = glm::mat4(1.0f);
				model2 = glm::translate(model2, glm::vec3(
					(col - (nrColumns / 2)) * spacing,
					(row - (nrRows / 2)) * spacing,
					0.0f
				));
				shader->setMat4("model", model2);
				renderSphere();
			}
		}

		// render light source (simply re-render sphere at light positions)
		// this looks a bit off as we use the same shader, but it'll make their positions obvious and 
		// keeps the codeprint small.
		for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
		{
			glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
			newPos = lightPositions[i];
			shader->setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
			shader->setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

			model2 = glm::mat4(1.0f);
			model2 = glm::translate(model2, newPos);
			model2 = glm::scale(model2, glm::vec3(0.5f));
			shader->setMat4("model", model2);
			renderSphere();
		}

#endif // 0
	}

#ifdef PBRirradiance_conversion

	void GameObject::PBRirradiance_conversion()
	{
		pbrShader->use();
		pbrShader->setVec3("albedo", 0.5f, 0.0f, 0.0f);
		pbrShader->setFloat("ao", 1.0f);

		backgroundShader->use();
		backgroundShader->setInt("environmentMap", 0);


		// lights
		// ------
		glm::vec3 lightPositions[] = {
			glm::vec3(-10.0f,  10.0f, 10.0f),
			glm::vec3(10.0f,  10.0f, 10.0f),
			glm::vec3(-10.0f, -10.0f, 10.0f),
			glm::vec3(10.0f, -10.0f, 10.0f),
		};
		glm::vec3 lightColors[] = {
			glm::vec3(300.0f, 300.0f, 300.0f),
			glm::vec3(300.0f, 300.0f, 300.0f),
			glm::vec3(300.0f, 300.0f, 300.0f),
			glm::vec3(300.0f, 300.0f, 300.0f)
		};
		int nrRows = 7;
		int nrColumns = 7;
		float spacing = 2.5;

		// pbr: setup framebuffer
		// ----------------------
		unsigned int captureFBO;
		unsigned int captureRBO;
		glGenFramebuffers(1, &captureFBO);
		glGenRenderbuffers(1, &captureRBO);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

		// pbr: load the HDR environment map
		// ---------------------------------
		stbi_set_flip_vertically_on_load(true);
		int width, height, nrComponents;
		float* data = stbi_loadf("assets/tex/hdr/newport_loft.hdr", &width, &height, &nrComponents, 0);
		unsigned int hdrTexture;
		if (data)
		{
			glGenTextures(1, &hdrTexture);
			glBindTexture(GL_TEXTURE_2D, hdrTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load HDR image." << std::endl;
		}

		// pbr: setup cubemap to render to and attach to framebuffer
		// ---------------------------------------------------------
		unsigned int envCubemap;
		glGenTextures(1, &envCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
		// ----------------------------------------------------------------------------------------------
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		// pbr: convert HDR equirectangular environment map to cubemap equivalent
		// ----------------------------------------------------------------------
		equirectangularToCubemapShader->use();
		equirectangularToCubemapShader->setInt("equirectangularMap", 0);
		equirectangularToCubemapShader->setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);

		glViewport(0, 0, Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight()); // don't forget to configure the viewport to the capture dimensions.
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		for (unsigned int i = 0; i < 6; ++i)
		{
			equirectangularToCubemapShader->setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			renderCube();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// initialize static shader uniforms before rendering
		// --------------------------------------------------
		glm::mat4 projection2 = glm::perspective(glm::radians(Application::Get().m_gameLayer->m_3DCamera.Zoom), (float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight(), 0.1f, 100.0f);
		pbrShader->use();
		pbrShader->setMat4("projection", projection2);
		backgroundShader->use();
		backgroundShader->setMat4("projection", projection2);


		// render scene, supplying the convoluted irradiance map to the final shader.
	   // ------------------------------------------------------------------------------------------
		pbrShader->use();
		glm::mat4 view = Application::Get().m_gameLayer->m_3DCamera.GetViewMatrix();
		pbrShader->setMat4("view", view);
		pbrShader->setVec3("camPos", Application::Get().m_gameLayer->m_3DCamera.Position);

		// render rows*column number of spheres with material properties defined by textures (they all have the same material properties)
		glm::mat4 model = glm::mat4(1.0f);
		for (int row = 0; row < nrRows; ++row)
		{
			pbrShader->setFloat("metallic", (float)row / (float)nrRows);
			for (int col = 0; col < nrColumns; ++col)
			{
				// we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
				// on direct lighting.
				pbrShader->setFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(
					(float)(col - (nrColumns / 2)) * spacing,
					(float)(row - (nrRows / 2)) * spacing,
					-2.0f
				));
				pbrShader->setMat4("model", model);
				renderSphere();
			}
		}


		// render light source (simply re-render sphere at light positions)
		// this looks a bit off as we use the same shader, but it'll make their positions obvious and 
		// keeps the codeprint small.
		for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
		{
			glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
			newPos = lightPositions[i];
			pbrShader->setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
			pbrShader->setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

			model = glm::mat4(1.0f);
			model = glm::translate(model, newPos);
			model = glm::scale(model, glm::vec3(0.5f));
			pbrShader->setMat4("model", model);
			renderSphere();
		}

		// render skybox (render as last to prevent overdraw)
		backgroundShader->use();
		backgroundShader->setMat4("view", view);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		renderCube();


	}
#endif // PBRirradiance_conversion

	// utility function for loading a 2D texture from file
	// ---------------------------------------------------
	unsigned int  GameObject::loadTexture(char const* path)
	{
		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
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

	void GameObject::renderCube()
	{
		// initialize (if necessary)
		if (cubeVAO == 0)
		{
			float vertices[] = {
				// back face
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
				 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
				// front face
				-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
				 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
				 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
				 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
				-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
				-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
				// left face
				-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
				-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
				-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
				-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
				// right face
				 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
				 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
				 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
				 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
				 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
				 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
				// bottom face
				-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
				 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
				 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
				 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
				-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
				-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
				// top face
				-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
				 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
				 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
				 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
				-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
				-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
			};
			glGenVertexArrays(1, &cubeVAO);
			glGenBuffers(1, &cubeVBO);
			// fill buffer
			glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			// link vertex attributes
			glBindVertexArray(cubeVAO);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
		// render Cube
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}


}

#endif