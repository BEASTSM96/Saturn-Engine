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

#pragma once

#include "SingletonStorage.h"

#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <Windows.h>

namespace Saturn {

	class RenderThread
	{
	public:
		static inline RenderThread& Get() { return *SingletonStorage::Get().GetOrCreateSingleton<RenderThread>(); }
	public:
		RenderThread();
		~RenderThread();

		void WaitAll();

		// Executes the most recent command.
		void ExecuteOne();

		void Terminate();

		template<typename Fn, typename... Args>
		void Queue( Fn&& rrFunc, Args&&... rrArgs ) 
		{
			std::unique_lock<std::mutex> Lock( m_Mutex, std::try_to_lock );
			m_QueueCV.notify_one();

			m_CommandBuffer.push_back( std::move( rrFunc ) );
		}

		float GetWaitTime() { return m_WaitTime.ElapsedMilliseconds(); }

		bool IsRenderThread();

		void Signal() { return m_SignalCV.notify_one(); }

		void Block() { m_Blocked = true; Signal(); m_QueueCV.notify_one(); }
		void Unblock() { m_Blocked = false; }
		
	private:
		void ThreadRun();
	private:
		bool m_ExecuteAll = false;
		bool m_ExecuteOne = false;

		// TEMP: This should be reworked.
		bool m_Blocked = false;

		Timer m_WaitTime;

		std::thread m_Thread;
		std::thread::id m_ThreadID;
		std::mutex m_Mutex;
		std::shared_ptr<std::atomic_bool> m_Running;

		std::condition_variable m_QueueCV;
		std::condition_variable m_SignalCV;

		std::vector<std::function<void()>> m_CommandBuffer;
	};
}