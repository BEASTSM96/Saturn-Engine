/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2021 BEAST																	*
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
#include "xGL.h"

#undef XGL_LOAD_FUNCTION

#define XGL_LOAD_FUNCTION( type, name ) type name = ( type ) XGL_EMPTY;

XGL_FUNCS_1_0
XGL_FUNCS_1_1
XGL_FUNCS_1_2
XGL_FUNCS_1_3
XGL_FUNCS_1_4
XGL_FUNCS_1_5
XGL_FUNCS_2_0
XGL_FUNCS_2_1
XGL_FUNCS_3_0
XGL_FUNCS_3_1
XGL_FUNCS_3_2
XGL_FUNCS_3_3
XGL_FUNCS_4_0
XGL_FUNCS_4_1
XGL_FUNCS_4_2
XGL_FUNCS_4_3
XGL_FUNCS_4_4
XGL_FUNCS_4_5
XGL_FUNCS_4_6

#undef XGL_LOAD_FUNCTION

#define XGL_LOAD_FUNCTION( type, name ) \
name = (type)xGL::LoadFunc( #name );

struct XGL_GL_VERSION_STRUCT { int major; int minor; };
XGL_GL_VERSION_STRUCT GLVersion;

void xGL::FindGLVersion()
{
	int i, ma, mi;

	const char* version;
	const char* prefixes[] ={
		"OpenGL ES-CM ",
		"OpenGL ES-CL ",
		"OpenGL ES ",
		NULL
	};

	version = ( const char* )glGetString( GL_VERSION );
	if( !version ) return;

	for( i = 0; prefixes[ i ]; i++ )
	{
		const size_t length = strlen( prefixes[ i ] );
		if( strncmp( version, prefixes[ i ], length ) == 0 )
		{
			version += length;
			break;
		}
	}

#ifdef _MSC_VER
	sscanf_s( version, "%d.%d", &ma, &mi );
#else
	sscanf( version, "%d.%d", &ma, &mi );
#endif

	GLVersion.major = ma;
	GLVersion.minor = mi;
}

const char* xGL::LibraryVersion()
{
	return XGL_VERSION;
}

const char* xGL::OpenGLVersion()
{
	return XGL_HIGHEST_SUPPORTED_VERSION;
}

const char* xGL::OpenGLString()
{
	return XGL_GL_STRING;
}

bool xGL::LoadGL()
{
	GLVersion.major = 0;
	GLVersion.minor = 0;

	// https://github.com/BEASTSM96/Saturn-Engine/blob/v2/Saturn/vendor/Glad/src/glad.c#L1805

	glGetString = ( PFNGLGETSTRINGPROC )LoadFunc( "glGetString" );

	if( glGetString == NULL ) return false;
	if( glGetString( GL_VERSION ) == NULL ) return false;

	FindGLVersion();

	XGL_FUNCS_1_0
	XGL_FUNCS_1_1
	XGL_FUNCS_1_2
	XGL_FUNCS_1_3
	XGL_FUNCS_1_4
	XGL_FUNCS_1_5
	XGL_FUNCS_2_0
	XGL_FUNCS_2_1
	XGL_FUNCS_3_0
	XGL_FUNCS_3_1
	XGL_FUNCS_3_2
	XGL_FUNCS_3_3
	XGL_FUNCS_4_0
	XGL_FUNCS_4_1
	XGL_FUNCS_4_2
	XGL_FUNCS_4_3
	XGL_FUNCS_4_4
	XGL_FUNCS_4_5
	XGL_FUNCS_4_6

	return GLVersion.major != 0 || GLVersion.minor != 0;
}

#if defined ( _WIN32 )

static HMODULE s_Handle = NULL;

typedef PROC( *PFNwglGetProcAddress )( LPCSTR args );

void* xGL::LoadFunc( const char* name )
{
	static PFNwglGetProcAddress wglGetProcAddress = NULL;

	if( !s_Handle )
	{
		s_Handle = LoadLibraryA( "opengl32.dll" );
		wglGetProcAddress = ( PFNwglGetProcAddress )GetProcAddress( s_Handle, "wglGetProcAddress" );
	}

	// https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions

	void* func = ( void* )wglGetProcAddress( name );
	if( func == 0 ||
	  ( func == ( void* )0x1 ) || ( func == ( void* )0x2 ) || ( func == ( void* )0x3 ) ||
	  ( func == ( void* )-1 ) )
	{
		HMODULE module = LoadLibraryA( "opengl32.dll" );
		func = ( void* )GetProcAddress( s_Handle, name );
	}

	return func;
}

void xGL::Terminate()
{
	FreeLibrary( s_Handle );
	s_Handle = NULL;
}

#elif defined ( __linux__ )

#include <dlfcn.h>

static void* s_Handle = XGL_EMPTY;

void* xGL::LoadFunc( const char* name )
{
	if( !s_Handle )
	{
		s_Handle = dlopen( " libGL.so.1", RTLD_LAZY | RTLD_LOCAL );
		if( !s_Handle )
		{
			s_Handle = dlopen( " libGL.so", RTLD_LAZY | RTLD_LOCAL );
		}
	}

	void* func = dlsym( s_Handle, name );

	return func;
}

void xGL::Terminate()
{
	if( s_Handle )
	{
		dlclose( s_Handle );
		s_Handle = NULL;
	}
}

#else // __linux__

#warning "OpenGL was deprecated in macOS 10.14. To create high performance code on GPUs, use the Metal framework instead."

#include <dlfcn.h>

static void* s_Handle;

void* xGL::LoadFunc( const char* name )
{
	static const char* libraryNames[] ={
		"../Frameworks/OpenGL.framework/OpenGL",
		"/Library/Frameworks/OpenGL.framework/OpenGL",
		"/System/Library/Frameworks/OpenGL.framework/OpenGL",
		"/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL"
	};

	// https://github.com/BEASTSM96/Saturn-Engine/blob/v2/Saturn/vendor/Glad/src/glad.c#L101
	unsigned int index = 0;
	for( index = 0; index < ( sizeof( libraryNames ) / sizeof( libraryNames[ 0 ] ) ); index++ )
	{
		s_Handle = dlopen( libraryNames[ index ], RTLD_NOW | RTLD_GLOBAL );

		if( s_Handle != NULL )
		{
			void* fn = dlsym( s_Handle, name );

			return fn;
		}
	}

	return 0; // Unable to find the OpenGL files
}

void xGL::Terminate()
{
	if( !s_Handle )
	{
		dlclose( s_Handle );
		s_Handle = NULL;
	}
}

#endif // __APPLE__
