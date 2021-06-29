/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
*                                                                                           *
* Permission is hereby granted, free of charge, to any person obtaining a copy              *
* of this software and associated documentation files (the "Software"), to deal             *
* in the Software without restriction, including without limitation the rights              *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                 *
* copies of the Software, and to permit persons to whom the Software is                     *
* furnished to do so, subject to the following conditions:                                  *
*                                                                                           *
* The above copyright notice and this permission notice shall be included in all            *
* copies or substantial portions of the Software.                                           *
*                                                                                           *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                  *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE               *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                    *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,             *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE             *
* SOFTWARE.                                                                                 *
*********************************************************************************************
*/

#include "sppch.h"
#include "OpenGLShadowFBO.h"

#include "Saturn/Renderer/Renderer.h"

namespace Saturn {

	OpenGLShadowMapFBO::OpenGLShadowMapFBO( int width, int height )
	{
		Resize( width, height );
	}

	OpenGLShadowMapFBO::~OpenGLShadowMapFBO()
	{

	}

	void OpenGLShadowMapFBO::CreateBuffer()
	{
		Renderer::Submit( [=]()
		{
			glGenFramebuffers( 1, &m_Fbo );

			glGenTextures( 1, &m_ShadowMap );
			glBindTexture( GL_TEXTURE_2D, m_ShadowMap );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glBindFramebuffer( GL_FRAMEBUFFER, m_Fbo );
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_ShadowMap, 0 );
			glDrawBuffer( GL_NONE );
			glReadBuffer( GL_NONE );
			glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		} );
	}

	void OpenGLShadowMapFBO::AttachBuffer()
	{	
		
	}

	void OpenGLShadowMapFBO::Bind()
	{
		Renderer::Submit( [=]()
		{
			glBindFramebuffer( GL_FRAMEBUFFER, m_Fbo );
			glClear( GL_DEPTH_BUFFER_BIT );
			glActiveTexture( GL_TEXTURE0 );
		} );
	}

	void OpenGLShadowMapFBO::Unbind()
	{
		Renderer::Submit( [=]()
		{
			glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		} );
	}

	void OpenGLShadowMapFBO::Resize( int width, int height )
	{
		Renderer::Submit( [=]()
		{
			if( m_Fbo )
			{
				glDeleteFramebuffers( 1, &m_Fbo );
				glDeleteTextures( 1, &m_ShadowMap );
			}

			glGenFramebuffers( 1, &m_Fbo );

			glGenTextures( 1, &m_ShadowMap );
			glBindTexture( GL_TEXTURE_2D, m_ShadowMap );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glBindFramebuffer( GL_FRAMEBUFFER, m_Fbo );
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_ShadowMap, 0 );
			glDrawBuffer( GL_NONE );
			glReadBuffer( GL_NONE );
			glBindFramebuffer( GL_FRAMEBUFFER, 0 );

		} );
	}

}