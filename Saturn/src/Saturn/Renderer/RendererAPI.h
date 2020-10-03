#pragma once

namespace Saturn {
<<<<<<< HEAD

	using RendererID = uint32_t;

	enum class RendererAPIType
=======
	class SATURN_API RendererAPI
>>>>>>> parent of 0ef25b2... TestCommit
	{
		None,
		OpenGL
	};

	// TODO: move into separate header
	enum class PrimitiveType
	{
		None = 0, Triangles, Lines
	};

	struct RenderAPICapabilities
	{
		std::string Vendor;
		std::string Renderer;
		std::string Version;

		int MaxSamples = 0;
		float MaxAnisotropy = 0.0f;
		int MaxTextureUnits = 0;
	};

	class RendererAPI
	{
	private:

	public:
		static void Init();
		static void Shutdown();

		static void Clear(float r, float g, float b, float a);
		static void SetClearColor(float r, float g, float b, float a);

<<<<<<< HEAD
		static void DrawIndexed(uint32_t count, PrimitiveType type, bool depthTest = true);
		static void SetLineThickness(float thickness);
=======
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) = 0;
>>>>>>> parent of 0ef25b2... TestCommit

		static RenderAPICapabilities& GetCapabilities()
		{
			static RenderAPICapabilities capabilities;
			return capabilities;
		}

		static RendererAPIType Current() { return s_CurrentRendererAPI; }
	private:
		static void LoadRequiredAssets();
	private:
		static RendererAPIType s_CurrentRendererAPI;
	};

}