/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2024 BEAST                                                           *
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

#include "Saturn/Asset/AssetRegistry.h"

namespace Saturn {

	Sound2D::Sound2D()
		: Sound()
	{
		m_RawPath = Project::GetActiveProject()->FilepathAbs( Path );
	}

	void Sound2D::Load()
	{
		if( !m_Loaded )
		{
			SAT_CORE_INFO( "Loading sound: {0}", m_RawPath.string() );

			ma_uint32 flags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_ASYNC;
			MA_CHECK( ma_sound_init_from_file( &AudioSystem::Get().GetAudioEngine(), 
				m_RawPath.string().c_str(),
				flags, NULL, NULL, &m_Sound ) );

			m_Sound.pEndCallbackUserData = reinterpret_cast< void* >( static_cast< intptr_t >( ID ) );
			m_Sound.endCallback = OnSoundEnd;

			m_Loaded = true;
		}
	}

	Sound2D::~Sound2D()
	{
		Stop();
		ma_sound_uninit( &m_Sound );
		m_Loaded = false;
	}

	void Sound2D::Play()
	{
		if( !m_Loaded )
			Load();

		SAT_CORE_INFO( "Trying to start sound: {0}", m_RawPath.string() );

		MA_CHECK( ma_sound_start( &m_Sound ) );

		m_Playing = true;
	}

	void Sound2D::Stop()
	{
		MA_CHECK( ma_sound_stop( &m_Sound ) );
		m_Playing = false;
	}

	void Sound2D::Loop()
	{
		ma_sound_set_looping( &m_Sound, true );
		m_Looping = true;
	}

	bool Sound2D::IsPlaying()
	{
		return m_Playing;
	}

	bool Sound2D::IsLooping()
	{
		return m_Looping;
	}

	void Sound2D::OnSoundEnd( void* pUserData, ma_sound* pSound )
	{
		AssetID ID = static_cast<uint64_t>( reinterpret_cast<intptr_t>( pUserData ) );

		AudioSystem::Get().ReportSoundCompleted( ID );
	}

}