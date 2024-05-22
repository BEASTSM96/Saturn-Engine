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

#pragma once

#include "SingletonStorage.h"
#include "Saturn/Core/Thread.h"

#include "AudioCore.h"
#include "Sound.h"

#include <filesystem>

namespace Saturn {

	enum AudioType
	{
		Static, // 2D
		Dynamic, // 3D
		Unknown
	};

	class AudioThread : public Thread
	{
	public:
		AudioThread();
		virtual ~AudioThread();

	public:
		virtual void Start() override;
		virtual void RequestJoin() override;

	private:
		void ThreadRun();

		bool m_PendingTerminate = false;
	};

	class Sound2D;

	class AudioSystem
	{
	public:
		static inline AudioSystem& Get() { return *SingletonStorage::GetOrCreateSingleton<AudioSystem>(); }
	public:
		AudioSystem();
		~AudioSystem();

		void Terminate();

		// Loads a new Sound2D to be played
		// By default the sound will automatically play upon loading.
		// WARNING:
		//  This function uses the Audio Thread
		//  This function will return the sound however, it may not be loaded as soon as it returns!
		//  Use WaitUntilLoaded for safety.
		Ref<Sound2D> RequestNewSound( AssetID ID, bool Play = true );
		
		void ReportSoundCompleted( AssetID ID );

		ma_engine& GetAudioEngine() { return m_Engine; }

	private:
		void Initialise();
		void StartNewSound( Ref<Sound> rSoundAsset );
		void PlaySound( Ref<Sound> rSoundAsset );

	private:
		Ref<AudioThread> m_AudioThread;

		// Currently alive sounds (i.e. sounds that are playing)
		std::unordered_map<AssetID, Ref<Sound>> m_AliveSounds;
		// A list of every loaded sound in memory
		std::unordered_map<AssetID, Ref<Sound>> m_LoadedSounds;

	private:
		ma_engine m_Engine;
		ma_context m_Context;
		ma_device m_Device;
	};
}