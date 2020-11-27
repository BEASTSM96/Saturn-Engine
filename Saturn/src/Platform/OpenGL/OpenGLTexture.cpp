#include "sppch.h"
#include "OpenGLTexture.h"

#include "Saturn/Renderer/RendererAPI.h"
#include "Saturn/Renderer/Renderer.h"

#include "stb_image.h"
#include <glad/glad.h>

namespace Saturn {
	static GLenum SaturnToOpenGLTextureFormat(TextureFormat format)
	{
		switch (format)
		{
		case Saturn::TextureFormat::RGB:     return GL_RGB;
		case Saturn::TextureFormat::RGBA:    return GL_RGBA;
		case Saturn::TextureFormat::Float16: return GL_RGBA16F;
		}
		SAT_CORE_ASSERT(false, "Unknown texture format!");
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Texture2D
	//////////////////////////////////////////////////////////////////////////////////

	OpenGLTexture2D::OpenGLTexture2D(TextureFormat format, u32 width, u32 height, TextureWrap wrap)
		: m_Format(format), m_Width(width), m_Height(height), m_Wrap(wrap)
	{
		Ref<OpenGLTexture2D> instance = this;
		Renderer::Submit([instance]() mutable
		{
			glGenTextures(1, &instance->m_RendererID);
			glBindTexture(GL_TEXTURE_2D, instance->m_RendererID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			GLenum wrap = instance->m_Wrap == TextureWrap::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
			glTextureParameterf(instance->m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, RendererAPI::GetCapabilities().MaxAnisotropy);

			glTexImage2D(GL_TEXTURE_2D, 0, SaturnToOpenGLTextureFormat(instance->m_Format), instance->m_Width, instance->m_Height, 0, SaturnToOpenGLTextureFormat(instance->m_Format), GL_UNSIGNED_BYTE, nullptr);

			glBindTexture(GL_TEXTURE_2D, 0);
		});

		m_ImageData.Allocate(width * height * Texture::GetBPP(m_Format));
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool srgb)
		: m_FilePath(path)
	{
		stbi_set_flip_vertically_on_load(false);

		int width, height, channels;
		if (stbi_is_hdr(path.c_str()))
		{
			SAT_CORE_INFO("Loading HDR texture {0}, srgb={1}", path, srgb);
			m_ImageData.Data = (byte*)stbi_loadf(path.c_str(), &width, &height, &channels, 0);
			m_IsHDR = true;
			m_Format = TextureFormat::Float16;
		}
		else
		{
			SAT_CORE_INFO("Loading texture {0}, srgb={1}", path, srgb);
			m_ImageData.Data = stbi_load(path.c_str(), &width, &height, &channels, srgb ? STBI_rgb : STBI_rgb_alpha);
			SAT_CORE_ASSERT(m_ImageData.Data, "Could not read image!");
			m_Format = TextureFormat::RGBA;
		}

		if (!m_ImageData.Data)
			return;

		m_Loaded = true;

		m_Width = width;
		m_Height = height;

		Ref<OpenGLTexture2D> instance = this;
		Renderer::Submit([instance, srgb]() mutable
		{
			// TODO: Consolidate properly
			if (srgb)
			{
				glCreateTextures(GL_TEXTURE_2D, 1, &instance->m_RendererID);
				int levels = Texture::CalculateMipMapCount(instance->m_Width, instance->m_Height);
				glTextureStorage2D(instance->m_RendererID, levels, GL_SRGB8, instance->m_Width, instance->m_Height);
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
				glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glTextureSubImage2D(instance->m_RendererID, 0, 0, 0, instance->m_Width, instance->m_Height, GL_RGB, GL_UNSIGNED_BYTE, instance->m_ImageData.Data);
				glGenerateTextureMipmap(instance->m_RendererID);
			}
			else
			{
				glGenTextures(1, &instance->m_RendererID);
				glBindTexture(GL_TEXTURE_2D, instance->m_RendererID);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

				GLenum internalFormat = SaturnToOpenGLTextureFormat(instance->m_Format);
				GLenum format = srgb ? GL_SRGB8 : (instance->m_IsHDR ? GL_RGB : SaturnToOpenGLTextureFormat(instance->m_Format)); // HDR = GL_RGB for now
				GLenum type = internalFormat == GL_RGBA16F ? GL_FLOAT : GL_UNSIGNED_BYTE;
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, instance->m_Width, instance->m_Height, 0, format, type, instance->m_ImageData.Data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glBindTexture(GL_TEXTURE_2D, 0);
			}
			stbi_image_free(instance->m_ImageData.Data);
		});
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		GLuint rendererID = m_RendererID;
		Renderer::Submit([rendererID]() {
			glDeleteTextures(1, &rendererID);
		});
	}

	void OpenGLTexture2D::Bind(u32 slot) const
	{
		Ref<const OpenGLTexture2D> instance = this;
		Renderer::Submit([instance, slot]() {
			glBindTextureUnit(slot, instance->m_RendererID);
		});
	}

	void OpenGLTexture2D::Lock()
	{
		m_Locked = true;
	}

	void OpenGLTexture2D::Unlock()
	{
		m_Locked = false;
		Ref<OpenGLTexture2D> instance = this;
		Renderer::Submit([instance]() {
			glTextureSubImage2D(instance->m_RendererID, 0, 0, 0, instance->m_Width, instance->m_Height, SaturnToOpenGLTextureFormat(instance->m_Format), GL_UNSIGNED_BYTE, instance->m_ImageData.Data);
		});
	}

	void OpenGLTexture2D::Resize(u32 width, u32 height)
	{
		SAT_CORE_ASSERT(m_Locked, "Texture must be locked!");

		m_ImageData.Allocate(width * height * Texture::GetBPP(m_Format));
#if SAT_DEBUG
		m_ImageData.ZeroInitialize();
#endif
	}

	Buffer OpenGLTexture2D::GetWriteableBuffer()
	{
		SAT_CORE_ASSERT(m_Locked, "Texture must be locked!");
		return m_ImageData;
	}

	u32 OpenGLTexture2D::GetMipLevelCount() const
	{
		return Texture::CalculateMipMapCount(m_Width, m_Height);
	}

	//////////////////////////////////////////////////////////////////////////////////
	// TextureCube
	//////////////////////////////////////////////////////////////////////////////////

	OpenGLTextureCube::OpenGLTextureCube(TextureFormat format, u32 width, u32 height)
	{
		m_Width = width;
		m_Height = height;
		m_Format = format;

		u32 levels = Texture::CalculateMipMapCount(width, height);
		Ref<OpenGLTextureCube> instance = this;
		Renderer::Submit([instance, levels]() mutable
		{
			glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &instance->m_RendererID);
			glTextureStorage2D(instance->m_RendererID, levels, SaturnToOpenGLTextureFormat(instance->m_Format), instance->m_Width, instance->m_Height);
			glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			// glTextureParameterf(m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, 16);
		});
	}

	// TODO: Revisit this, as currently env maps are being loaded as equirectangular 2D images
	//       so this is an old path
	OpenGLTextureCube::OpenGLTextureCube(const std::string& path)
		: m_FilePath(path)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(false);
		m_ImageData = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb);

		m_Width = width;
		m_Height = height;
		m_Format = TextureFormat::RGB;

		u32 faceWidth = m_Width / 4;
		u32 faceHeight = m_Height / 3;
		SAT_CORE_ASSERT(faceWidth == faceHeight, "Non-square faces!");

		std::array<u8*, 6> faces;
		for (size_t i = 0; i < faces.size(); i++)
			faces[i] = new u8[faceWidth * faceHeight * 3]; // 3 BPP

		int faceIndex = 0;

		for (size_t i = 0; i < 4; i++)
		{
			for (size_t y = 0; y < faceHeight; y++)
			{
				size_t yOffset = y + faceHeight;
				for (size_t x = 0; x < faceWidth; x++)
				{
					size_t xOffset = x + i * faceWidth;
					faces[faceIndex][(x + y * faceWidth) * 3 + 0] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 0];
					faces[faceIndex][(x + y * faceWidth) * 3 + 1] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 1];
					faces[faceIndex][(x + y * faceWidth) * 3 + 2] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 2];
				}
			}
			faceIndex++;
		}

		for (size_t i = 0; i < 3; i++)
		{
			// Skip the middle one
			if (i == 1)
				continue;

			for (size_t y = 0; y < faceHeight; y++)
			{
				size_t yOffset = y + i * faceHeight;
				for (size_t x = 0; x < faceWidth; x++)
				{
					size_t xOffset = x + faceWidth;
					faces[faceIndex][(x + y * faceWidth) * 3 + 0] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 0];
					faces[faceIndex][(x + y * faceWidth) * 3 + 1] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 1];
					faces[faceIndex][(x + y * faceWidth) * 3 + 2] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 2];
				}
			}
			faceIndex++;
		}

		Ref<OpenGLTextureCube> instance = this;
		Renderer::Submit([instance, faceWidth, faceHeight, faces]() mutable
		{
			glGenTextures(1, &instance->m_RendererID);
			glBindTexture(GL_TEXTURE_CUBE_MAP, instance->m_RendererID);

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTextureParameterf(instance->m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, RendererAPI::GetCapabilities().MaxAnisotropy);

			auto format = SaturnToOpenGLTextureFormat(instance->m_Format);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[2]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[0]);

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[4]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[5]);

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[1]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[3]);

			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

			glBindTexture(GL_TEXTURE_2D, 0);

			for (size_t i = 0; i < faces.size(); i++)
				delete[] faces[i];

			stbi_image_free(instance->m_ImageData);
		});
	}

	OpenGLTextureCube::~OpenGLTextureCube()
	{
		GLuint rendererID = m_RendererID;
		Renderer::Submit([rendererID]() {
			glDeleteTextures(1, &rendererID);
		});
	}

	void OpenGLTextureCube::Bind(u32 slot) const
	{
		Ref<const OpenGLTextureCube> instance = this;
		Renderer::Submit([instance, slot]() {
			glBindTextureUnit(slot, instance->m_RendererID);
		});
	}

	u32 OpenGLTextureCube::GetMipLevelCount() const
	{
		return Texture::CalculateMipMapCount(m_Width, m_Height);
	}
}