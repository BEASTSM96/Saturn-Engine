/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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
#include "Sound2D.h"

namespace Saturn {

	Sound2D::Sound2D()
		: Sound()
	{
		Load();
	}

	void Sound2D::Load()
	{
		ma_uint32 flags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION;
		MA_CHECK( ma_sound_init_from_file( &AudioSystem::Get().GetAudioEngine(), m_RawPath.string().c_str(), flags, NULL, NULL, &m_Sound ) );
	}

	Sound2D::~Sound2D()
	{

	}

	void Sound2D::TryPlay()
	{
		m_Playing = AudioSystem::Get().Play( m_Sound );
	}

	void Sound2D::Stop()
	{
		AudioSystem::Get().Stop( m_Sound );
		m_Playing = false;
	}

	void Sound2D::Loop()
	{
		ma_sound_set_looping( &m_Sound, true );
	}

	bool Sound2D::IsPlaying()
	{
		return m_Playing;
	}

	bool Sound2D::IsLooping()
	{
		return true;
	}
}