#pragma once

namespace Saturn {

	using RendererID = uint32_t;

	enum class RendererAPIType
	{
		None = 0x0,
		OpenGL = 0x1,
		DX = 0x2,
		Vulkan = 0x4
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

	class SATURN_API RendererAPI
	{
	public:
		static void Init( void ) ;
		static void Shutdown( void ) ;

		static void Clear(float r, float g, float b, float a);
		static void SetClearColor(float r, float g, float b, float a);

		static void DrawIndexed(uint32_t count, PrimitiveType type, bool depthTest = true);
		static void SetLineThickness(float thickness);

		static RenderAPICapabilities& GetCapabilities()
		{
			static RenderAPICapabilities capabilities;
			return capabilities;
		}

		static RendererAPIType Current() { return s_CurrentRendererAPI; }
	private:
		static void LoadRequiredAssets( void ) ;
	private:
		static RendererAPIType s_CurrentRendererAPI;
	};
}