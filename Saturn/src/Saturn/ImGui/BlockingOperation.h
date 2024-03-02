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

#include "Saturn/Core/Timer.h"

#include <string>
#include <functional>

namespace Saturn {

	class BlockingOperation : public RefTarget
	{
	public:
		BlockingOperation();
		~BlockingOperation();

		float GetProgress() { return m_Progress; }
		const std::string& GetStatus() const { return m_Status; }
		const std::string& GetTitle() const { return m_Title; }

		void OnComplete( std::function<void()>&& rrFunction )
		{
			m_ExitFunction = rrFunction;
		}

		void SetJob( std::function<void()>&& rFunction );

		bool HasJob() { return m_Job != nullptr; }

		void Execute();

		void SetStatus( const std::string& rStatus ) { m_Status = rStatus; }
		void SetTitle( const std::string& rTitle ) { m_Title = rTitle; }

		void SetProgress( float progress ) { m_Progress = progress; }

	private:
		void ThreadRun();

	protected:
		float m_Progress = 0.0f;
		std::string m_Status;
		std::string m_Title;

		bool m_Done = false;
		Timer m_Duration;

	private:
		std::thread m_JobThread;
		std::function<void()> m_Job;
		std::function<void()> m_ExitFunction;
	};
}