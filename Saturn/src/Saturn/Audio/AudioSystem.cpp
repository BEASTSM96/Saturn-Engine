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

				m_MasterSoundGroup = Ref<SoundGroup>::Create( "Master" );
				m_MasterSoundGroup->Init( true );
			} );
	}

	void AudioSystem::Terminate()
	{
		// Stop and unload any alive sounds
		for( auto& [id, sound] : m_AliveSounds )
		{
			sound->Stop();
			sound->Unload();
		}

#if defined( SAT_DEBUG ) || defined( SAT_RELEASE )
		// Stop and unload any preview sounds
		for( auto& [identifier, identifierMap] : m_PreviewSounds )
		{
			for( auto& [ID, sound] : identifierMap ) 
			{
				sound->Stop();
				sound->Unload();
			}

			identifierMap.clear();
		}

		m_PreviewSounds.clear();
#endif

		// Unload remaining sounds
		for( auto& [id, sound] : m_LoadedSounds )
		{
			sound->Unload();
		}

		// Uninit project sound groups
		for( auto& rGroup : Project::GetActiveProject()->GetSoundGroups() )
		{
			rGroup->Destroy();
		}

		// And the master sound group as well
		m_MasterSoundGroup->Destroy();
		m_MasterSoundGroup = nullptr;

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

	void AudioSystem::PlaySound( Ref<Sound> soundAsset )
	{
		// TODO: Check if we are on the audio thread.
		soundAsset->Play();
	}

	Ref<Sound> AudioSystem::RequestNewSound( AssetID ID, UUID UniquePlayerID, bool Play /*= true */ )
	{
		// Load the sound spec.
		Ref<SoundSpecification> spec = AssetManager::Get().GetAssetAs<SoundSpecification>( ID );

		Ref<Sound> newSound = Ref<Sound>::Create( spec );
		m_AliveSounds[ UniquePlayerID ] = newSound;

		auto loadFunc = [=]() -> void
		{
			// Intentional.
			// Better to get the sound again rather than copy it into this lambda.
			Ref<Sound> newSound = m_AliveSounds[ UniquePlayerID ];

			newSound->Load( MA_SOUND_FLAG_NO_SPATIALIZATION );
			// If the sound was already loaded then we can still disable it here.
			newSound->SetSpatialization( false );
			newSound->SetID( UniquePlayerID );

			if( Play ) newSound->Play();

			m_LoadedSounds[ UniquePlayerID ] = newSound;
		};

		m_AudioThread->IsCurrentThread() ? loadFunc() : m_AudioThread->Queue( loadFunc );

		return newSound;
	}

	Ref<Sound> AudioSystem::PlaySoundAtLocation( AssetID ID, UUID UniquePlayerID, const glm::vec3& rPos, bool Play /*= true */ )
	{
		// Load the sound spec.
		Ref<SoundSpecification> spec = AssetManager::Get().GetAssetAs<SoundSpecification>( ID );

		Ref<Sound> newSound = Ref<Sound>::Create( spec );
		m_AliveSounds[ UniquePlayerID ] = newSound;

		auto loadFunc = [=]() -> void
		{
			// Intentional.
			// Better to get the sound again rather than copy it into this lambda.
			Ref<Sound> newSound = m_AliveSounds[ UniquePlayerID ];

			newSound->Load();
			// If the sound was already loaded then we can still enable it here.
			newSound->SetSpatialization( true );
			newSound->SetPosition( rPos );
			newSound->SetID( UniquePlayerID );

			if( Play ) newSound->Play();

			m_LoadedSounds[ UniquePlayerID ] = newSound;
		};

		m_AudioThread->IsCurrentThread() ? loadFunc() : m_AudioThread->Queue( loadFunc );

		return newSound;
	}

	Ref<Sound> AudioSystem::RequestPreviewSound( AssetID AssetID, UUID Identifier, bool AllowMultiple )
	{
#if defined( SAT_DEBUG ) || defined( SAT_RELEASE )
		if( !AllowMultiple )
		{
			const auto identifierItr = m_PreviewSounds.find( Identifier );

			if( identifierItr != m_PreviewSounds.end() )
			{
				auto& rIdentifierMap = identifierItr->second;

				for( auto& [id, sound] : rIdentifierMap )
				{
					sound->Stop();
					sound->Reset();
				}

				rIdentifierMap.clear();
			}
		}
		
		Ref<SoundSpecification> soundSpec = AssetManager::Get().GetAssetAs<SoundSpecification>( AssetID );

		Ref<Sound> snd = Ref<Sound>::Create( soundSpec );
		m_PreviewSounds[ Identifier ][ AssetID ] = snd;

		auto loadFunc = [=]() 
		{
			// Intentional.
			Ref<Sound> sound = m_PreviewSounds[ Identifier ][ AssetID ];

			sound->Load( MA_SOUND_FLAG_NO_SPATIALIZATION );
			sound->SetSpatialization( false );
			sound->Play();

			m_AliveSounds[ AssetID ] = sound;
			m_LoadedSounds[ AssetID ] = sound;
		};

		m_AudioThread->IsCurrentThread() ? loadFunc() : m_AudioThread->Queue( loadFunc );

		return snd;
#else
		return nullptr;
#endif
	}

	Ref<GraphSound> AudioSystem::PlayGraphSound( AssetID ID, UUID UniquePlayerID )
	{
		Ref<GraphSound> snd = Ref<GraphSound>::Create( ID );
		snd->SetID( UniquePlayerID );

		m_AliveSounds[ UniquePlayerID ] = snd;

		auto loadFunc = [=]()
			{
				// Intentional.
				Ref<GraphSound> graphSoundAsset = m_AliveSounds[ UniquePlayerID ];

				graphSoundAsset->Load( MA_SOUND_FLAG_NO_SPATIALIZATION );
				graphSoundAsset->Play();

				m_LoadedSounds[ UniquePlayerID ] = graphSoundAsset;
			};

		m_AudioThread->IsCurrentThread() ? loadFunc() : m_AudioThread->Queue( loadFunc );

		return snd;
	}

	void AudioSystem::RequestNewSounds( std::vector<AssetID> Ids, std::vector<UUID> PlayerIds, std::function<void(Ref<Sound>)>&& rVistor )
	{
		// Copy vectors because they might be destroyed by the time the audio thread gets the this function.
		auto loadFunc = [copyIds = Ids, copyPlayerIds = PlayerIds, rVistor, this]()
			{
				size_t soundIndex = 0;
				for( const AssetID& rAssetID : copyIds )
				{
					Ref<Sound> snd = RequestNewSound( rAssetID, copyPlayerIds[soundIndex] );

					if( rVistor )
						rVistor( snd );

					soundIndex++;
				}
			};

		m_AudioThread->IsCurrentThread() ? loadFunc() : m_AudioThread->Queue( loadFunc );
	}

	void AudioSystem::ReportSoundCompleted( UUID UniquePlayerID )
	{
#if defined( SAT_DEBUG ) || defined( SAT_RELEASE )
		for( auto&& [identifier, identifierMap] : m_PreviewSounds )
		{
			auto PreviewItr = identifierMap.find( UniquePlayerID );

			if( PreviewItr != identifierMap.end() )
			{
				auto& rSnd = ( PreviewItr )->second;

				rSnd->Stop();
				rSnd->Reset();
				rSnd->Unload();

				identifierMap.erase( PreviewItr );
				m_PreviewSounds.erase( identifier );

				break;
			}
		}
#endif
		auto Itr = m_AliveSounds.find( UniquePlayerID );

		if( Itr != m_AliveSounds.end() ) 
		{
			auto& rSnd = ( Itr )->second;
			rSnd->Stop();

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
			asset->Play( 0 );
		}
	}

	void AudioSystem::SetPrimaryListenerPos( const glm::vec3& rPos )
	{
		ma_engine_listener_set_position( &m_Engine, 0, rPos.x, rPos.y, rPos.z );
	}

	void AudioSystem::StopPreviewSounds( UUID Identifier )
	{
#if defined( SAT_DEBUG ) || defined( SAT_RELEASE )
		const auto identifierItr = m_PreviewSounds.find( Identifier );

		if( identifierItr != m_PreviewSounds.end() )
		{
			auto& rIdentifierMap = identifierItr->second;

			for( auto& [id, sound] : rIdentifierMap )
			{
				sound->Stop();
				sound->Reset();
			}

			rIdentifierMap.clear();
		}
#endif
	}

	void AudioSystem::StopAndResetSound( UUID UniquePlayerID )
	{
		const auto Itr = std::find_if( m_LoadedSounds.begin(), m_LoadedSounds.end(), 
			[UniquePlayerID]( const auto& kv )
			{
				return kv.first == UniquePlayerID;
			} );

		if( Itr != m_LoadedSounds.end() )
		{
			auto& rSound = ( Itr->second );

			rSound->Stop();
			rSound->Reset();
		}
	}

	void AudioSystem::UnloadSound( Ref<SoundBase> sound )
	{
		auto playerID = sound->m_PlayerID;

		sound->Unload();

		m_LoadedSounds.erase( playerID );

		const auto Itr = std::find_if( m_AliveSounds.begin(), m_AliveSounds.end(), 
			[playerID](const auto& kv)
			{
				return kv.first == playerID;
			} );

		if( Itr != m_AliveSounds.end() )
			m_AliveSounds.erase( Itr );
	}

	SoundDecodedInformation AudioSystem::DecodeSound( const Ref<SoundSpecification>& rSpec )
	{
		SoundDecodedInformation decodedInformation{};
		
		ma_decoder decoder;
		ma_decoder_config config{};
		
		config = ma_decoder_config_init( ma_format_f32, 2, m_Engine.sampleRate );

		MA_CHECK( ma_decoder_init_file( rSpec->SoundSourcePath.string().data(), &config, &decoder ) );

		uint64_t frames = 0;
		MA_CHECK( ma_decoder_get_length_in_pcm_frames( &decoder, &frames ) );

		uint64_t bpf = ma_get_bytes_per_frame( decoder.outputFormat, decoder.outputChannels );
		size_t bufferSize = frames * bpf;

		decodedInformation.PCMFrameCount = frames;
		decodedInformation.BytesPerFrame = bpf;
		decodedInformation.Channels = decoder.outputChannels;
		decodedInformation.SampleRate = decoder.outputSampleRate;
		decodedInformation.Format = (int)decoder.outputFormat;

		Buffer TemporaryBuffer;
		TemporaryBuffer.Allocate( bufferSize );
		TemporaryBuffer.Zero_Memory();

		uint64_t totalFrameRead = 0;
		MA_CHECK( ma_decoder_read_pcm_frames( &decoder, TemporaryBuffer.Data, frames, &totalFrameRead ) );

		SAT_CORE_ASSERT( totalFrameRead == frames, "Audio decoder did not read the whole file/buffer!" );
	
		MA_CHECK( ma_decoder_uninit( &decoder ) );

		decodedInformation.PCMFrames = Buffer::Copy( TemporaryBuffer.Data, TemporaryBuffer.Size );
		TemporaryBuffer.Free();

		return decodedInformation;
	}

}