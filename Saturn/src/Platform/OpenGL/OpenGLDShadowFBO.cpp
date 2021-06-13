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
#include "OpenGLDShadowFBO.h"

#include "Saturn/Renderer/Renderer.h"

namespace Saturn {

	OpenGLShadowMapFBO::OpenGLShadowMapFBO( int width, int height )
	{
		Renderer::Submit( [=]()
		{
			glGenFramebuffers( 1, &m_Fbo );

			glGenTextures( 1, &m_ShadowMap );
			glBindTexture( GL_TEXTURE_2D, m_ShadowMap );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

			glBindFramebuffer( GL_FRAMEBUFFER, m_Fbo );

			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_ShadowMap, 0 );

			glDrawBuffer( GL_NONE );
			glReadBuffer( GL_NONE );

			SAT_CORE_ASSERT( glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!" );
			if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
			{
				SAT_CORE_ERROR( "Framebuffer is incomplete!" );
			}

		} );
	}

	OpenGLShadowMapFBO::~OpenGLShadowMapFBO()
	{

	}

	void OpenGLShadowMapFBO::BindForWriting()
	{
		Renderer::Submit( [=]()
		{
			glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_Fbo );
		} );
	}

	void OpenGLShadowMapFBO::BindForReading( void* textureUnit )
	{
		GLenum textUnit = ( GLenum )textureUnit;

		Renderer::Submit( [=]()
		{
			glActiveTexture( textUnit );
			glBindTexture( GL_TEXTURE_2D, m_ShadowMap );
		} );
	}
}