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
#include "AudioSystem.h"

#include "SoundGroup.h"

#include "Saturn/Asset/AssetManager.h"
#include "Saturn/Project/Project.h"
#include "Saturn/Core/OptickProfiler.h"

namespace Saturn {

	//////////////////////////////////////////////////////////////////////////

	AudioThread::AudioThread()
		: Thread()
	{
	}

	AudioThread::~AudioThread()
	{
	}

	void AudioThread::Start()
	{
		m_Running->store( true );
		m_Thread = std::thread( &AudioThread::ThreadRun, this );
	}

	void AudioThread::RequestJoin()
	{
		std::unique_lock<std::mutex> Lock( m_Mutex );
		
		m_Running->store( false );
		m_SignalCV.notify_one();
		
		Lock.unlock();
	}

	void AudioThread::ThreadRun()
	{
		SetThreadDescription( GetCurrentThread(), L"Audio Thread" );
		m_ThreadID = std::this_thread::get_id();

		while( true )
		{
			SAT_PF_THRD( "Audio Thread" );

			std::unique_lock<std::mutex> Lock( m_Mutex );
		
			// Wait for the queue to not be empty.
			m_QueueCV.wait( Lock, [this]
				{
					return !m_Running->load() || !m_CommandBuffer.empty();
				} );

			if( !m_Running->load() ) break;

			Lock.unlock();

			ExecuteCommands();

			m_QueueCV.notify_all();
		}

		m_Running->store( false );
	}

	//////////////////////////////////////////////////////////////////////////

	AudioSystem::AudioSystem()
	{
		Initialise();
	}

	void AudioSystem::Initialise()
	{
		SAT_CORE_INFO( "Starting Audio Thread...");
		m_AudioThread = Ref<AudioThread>::Create();
		m_AudioThread->Start();

		// Queue initialisation on newly created audio thread.
		m_AudioThread->Queue( 
			[&]() 
			{
				// Create engine
				MA_CHECK( ma_engine_init( nullptr, &m_Engine ) );

				ma_backend backends[ 1 ] = { ma_backend_wasapi };
				MA_CHECK( ma_context_init( backends, 1, nullptr, &m_Context ) );

				ma_device_info deviceInfo;
				MA_CHECK( ma_context_get_device_info( &m_Context, ma_device_type_playback, nullptr, &deviceInfo ) );

				ma_device_config deviceConfig = ma_device_config_init( ma_device_type_playback );
				deviceConfig.playback.format = ma_format_f32;
				deviceConfig.playback.channels = 2;
				deviceConfig.sampleRate = 48000;
				deviceConfig.dataCallback = nullptr;

				MA_CHECK( ma_device_init( &m_Context, &deviceConfig, &m_Device ) );

				SAT_CORE_INFO( "Audio Device information:" );
				SAT_CORE_INFO( " Using backend API: {0}", ma_get_backend_name( backends[0] ) );
				SAT_CORE_INFO( " Device Name: {0}", deviceInfo.name );
				SAT_CORE_INFO( " Is Primary: {0}", deviceInfo.isDefault );
				SAT_CORE_INFO( " Channels: {0}", m_Device.playback.channels );
				SAT_CORE_INFO( " Sample Rate: {0}", m_Device.playback.internalSampleRate );
				SAT_CORE_INFO( " Buffer Cap: {0}", m_Device.playback.intermediaryBufferCap );
				SAT_CORE_INFO( " Format: {0}", m_Device.playback.format );
				SAT_CORE_INFO( "==============" );

				m_MasterSoundGroup.Init( true );
				m_MasterSoundGroup.SetName( "Master" );
			} );
	}

	void AudioSystem::Terminate()
	{
		// Stop and unload any alive sounds
		for( auto& [id, asset] : m_AliveSounds )
		{
			asset->Stop();
			asset->Unload();
		}

		// Unload remaining sounds
		for( auto& [id, asset] : m_LoadedSounds )
		{
			asset->Unload();
		}

		// Uninit project sound groups
		for( auto& rGroup : Project::GetActiveProject()->GetSoundGroups() )
		{
			rGroup->Destroy();
		}

		// Unit master sound group
		m_MasterSoundGroup.Destroy();

		// Stop audio thread
		SAT_CORE_INFO( "Stoping Audio Thread..." );
		m_AudioThread->RequestJoin();

		MA_CHECK( ma_device_stop( &m_Device ) );

		MA_CHECK( ma_context_uninit( &m_Context ) );

		// NOTE: Device is owned by the engine, so it will uninit it for us.
		ma_engine_uninit( &m_Engine );

		m_AliveSounds.clear();
		m_LoadedSounds.clear();
	}

	AudioSystem::~AudioSystem()
	{
		// No cleanup done in destructor, application already terminated the audio system.
	}

	void AudioSystem::StartNewSound( Ref<Sound> soundAsset )
	{
		soundAsset->Load();
	}

	void AudioSystem::PlaySound( Ref<Sound> soundAsset )
	{
		// TODO: Check if we are on the audio thread.
		soundAsset->Play();
	}

	Ref<Sound> AudioSystem::RequestNewSound( AssetID ID, bool Play /*= true */ )
	{
		// Try load the asset on the main thread to return it.
		// Of course at this point the sound will just be created
		// and not loaded.
		Ref<Sound> snd = AssetManager::Get().GetAssetAs<Sound>( ID );

		m_AudioThread->Queue( [=]()
			{
				// Intentional.
				// Better to get the sound again rather than copy it into this lambda.
				Ref<Sound> soundAsset = AssetManager::Get().GetAssetAs<Sound>( ID );

				soundAsset->Load( MA_SOUND_FLAG_NO_SPATIALIZATION );
				// If the sound was already loaded then we can still disable it here.
				soundAsset->SetSpatialization( false );

				if( Play )
					soundAsset->Play();

				m_AliveSounds[ ID ] = soundAsset;
				m_LoadedSounds[ ID ] = soundAsset;
			} );

		return snd;
	}

	Ref<Sound> AudioSystem::PlaySoundAtLocation( AssetID ID, const glm::vec3& rPos, bool Play /*= true */ )
	{
		Ref<Sound> snd = AssetManager::Get().GetAssetAs<Sound>( ID );

		m_AudioThread->Queue( [=]()
			{
				// Intentional.
				// Better to get the sound again rather than copy it into this lambda.
				Ref<Sound> soundAsset = AssetManager::Get().GetAssetAs<Sound>( ID ).As<Sound>();

				soundAsset->Load();
				// If the sound was already loaded then we can still enable it here.
				soundAsset->SetSpatialization( true );

				soundAsset->SetPosition( rPos );

				if( Play )
					soundAsset->Play();

				m_AliveSounds[ ID ] = soundAsset;
				m_LoadedSounds[ ID ] = soundAsset;
			} );

		return snd;
	}

	void AudioSystem::ReportSoundCompleted( AssetID ID )
	{
		auto Itr = m_AliveSounds.find( ID );

		if( Itr != m_AliveSounds.end() ) 
		{
			auto& rSnd = ( Itr )->second;
			rSnd->m_Playing = false;

			m_AliveSounds.erase( Itr );
		}
	}

	void AudioSystem::Suspend()
	{
		// Stop and unload any alive sounds
		for( auto& [id, asset] : m_AliveSounds )
		{
			asset->Stop();
		}
	
		MA_CHECK( ma_device_stop( &m_Device ) );
	}

	void AudioSystem::Resume()
	{
		MA_CHECK( ma_device_start( &m_Device ) );

		for( auto& [id, asset] : m_AliveSounds )
		{
			asset->Play();
		}
	}

	void AudioSystem::SetPrimaryListenerPos( const glm::vec3& rPos )
	{
		ma_engine_listener_set_position( &m_Engine, 0, rPos.x, rPos.y, rPos.z );
	}

}