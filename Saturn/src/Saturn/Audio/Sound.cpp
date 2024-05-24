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
#include "Sound.h"

#include "AudioSystem.h"

namespace Saturn {

	Sound::Sound()
		: SoundBase()
	{
	}

	void Sound::Load( uint32_t flags )
	{
		if( !m_Loaded )
		{
			SAT_CORE_INFO( "Loading sound: {0}", m_RawPath.string() );

			ma_uint32 initFlags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC;
			initFlags |= flags;

			MA_CHECK( ma_sound_init_from_file( &AudioSystem::Get().GetAudioEngine(),
				m_RawPath.string().c_str(),
				initFlags, NULL, NULL, &m_Sound ) );

			m_Sound.pEndCallbackUserData = reinterpret_cast< void* >( static_cast< intptr_t >( ID ) );
			m_Sound.endCallback = OnSoundEnd;

			if( ( initFlags & ( uint32_t ) MA_SOUND_FLAG_NO_SPATIALIZATION ) == 0 )
			{
				m_Spatialization = true;
			}

			m_Loaded = true;
		}
	}

	void Sound::Unload()
	{
		if( m_Loaded )
		{
			ma_sound_uninit( &m_Sound );
			m_Loaded = false;
		}
	}

	Sound::~Sound()
	{
		Stop();
		Unload();
	}

	void Sound::Play()
	{
		if( !m_Loaded )
			Load( 0 );

		if( ma_sound_at_end( &m_Sound ) )
		{
			SAT_CORE_WARN( "Playing sound from beginning: {0}", m_RawPath.string() );
			Reset();
		}

		SAT_CORE_INFO( "Trying to start sound: {0}", m_RawPath.string() );

		MA_CHECK( ma_sound_start( &m_Sound ) );

		m_Playing = true;
	}

	void Sound::Stop()
	{
		MA_CHECK( ma_sound_stop( &m_Sound ) );
		m_Playing = false;
	}

	void Sound::Loop()
	{
		ma_sound_set_looping( &m_Sound, true );
		m_Looping = true;
	}

	bool Sound::IsPlaying() const
	{
		return m_Playing;
	}

	bool Sound::IsLooping() const
	{
		return m_Looping;
	}

	void Sound::WaitUntilLoaded()
	{
		while( !m_Loaded )
		{
			std::this_thread::yield();
		}
	}

	void Sound::Reset()
	{
		if( m_Loaded )
		{
			MA_CHECK( ma_sound_seek_to_pcm_frame( &m_Sound, 0 ) );
		}
	}

	void Sound::SetPosition( const glm::vec3& rPos )
	{
		if( m_Spatialization )
		{
			ma_sound_set_position( &m_Sound, rPos.x, rPos.y, rPos.z );
		}
	}

	void Sound::SetSpatialization( bool value )
	{
		if( m_Spatialization == value )
			return;

		ma_sound_set_spatialization_enabled( &m_Sound, value );
		m_Spatialization = value;
	}

	void Sound::OnSoundEnd( void* pUserData, ma_sound* pSound )
	{
		AssetID ID = static_cast< uint64_t >( reinterpret_cast< intptr_t >( pUserData ) );
		AudioSystem::Get().ReportSoundCompleted( ID );
	}
}