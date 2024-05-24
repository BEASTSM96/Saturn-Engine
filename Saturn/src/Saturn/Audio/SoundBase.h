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

#include "Saturn/Asset/Asset.h"
#include "AudioCore.h"

#include <miniaudio.h>
#include <filesystem>

namespace Saturn {

	class SoundBase : public Asset
	{
	public:
		SoundBase() = default;
		virtual ~SoundBase() = default;

		virtual void Play() = 0;
		virtual void Stop() = 0;
		virtual void Loop() = 0;
		virtual void Load( uint32_t flags ) = 0;

	public:
		ma_sound& GetRawSound() { return m_Sound; }

		const std::filesystem::path& GetRawPath() const { return m_RawPath; }
		std::filesystem::path& GetRawPath() { return m_RawPath; }

		void SetRawPath( const std::filesystem::path& rPath ) { m_RawPath = rPath; }

	protected:
		ma_sound m_Sound{};
		bool m_Loaded = false;
		bool m_Playing = false;
		bool m_Looping = false;

		std::filesystem::path m_RawPath = "";

	private:
		virtual void Unload() = 0;

	private:
		friend class AudioSystem;
	};
}