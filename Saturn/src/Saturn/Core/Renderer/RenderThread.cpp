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
#include "RenderThread.h"

#include "Saturn/Core/OptickProfiler.h"

namespace Saturn {

	RenderThread::RenderThread()
	{
		m_Running = std::make_shared<std::atomic_bool>();
		m_Running->store( true );

		m_Thread = std::thread( &RenderThread::ThreadRun, this );
	}

	RenderThread::~RenderThread()
	{

	}

	void RenderThread::WaitAll()
	{
		m_ExecuteAll = true;

		m_WaitTime.Reset();

		while( m_ExecuteAll )
		{
			std::this_thread::yield();
		}

		m_WaitTime.Stop();
	}

	void RenderThread::ExecuteOne()
	{
		m_ExecuteOne = true;
	}

	void RenderThread::Terminate()
	{
		if ( m_Thread.joinable() )
		{
			{
				std::lock_guard<std::mutex> Lock( m_Mutex );

				m_Running->store( false );

				m_Cond.notify_all();
			}

			while( m_Running->load() )
			{
				std::this_thread::yield();
			}

			m_Thread.join();
		}
	}

	bool RenderThread::IsRenderThread()
	{
		return std::this_thread::get_id() == m_ThreadID;
	}

	void RenderThread::ThreadRun()
	{
		SetThreadDescription( GetCurrentThread(), L"Render Thread" );
		m_ThreadID = std::this_thread::get_id();

		while (true)
		{
			SAT_PF_THRD( "Render Thread" );
			
			std::unique_lock<std::mutex> Lock( m_Mutex );

			m_Cond.wait( Lock, [this] 
				{
					return !m_Running->load() || !m_CommandBuffer.empty();
				} );

			if( !m_Running->load() ) break;

			Lock.unlock();

			if( m_ExecuteOne )
			{
				auto& rFunc = m_CommandBuffer.back();

				rFunc();

				m_CommandBuffer.pop_back();

				m_ExecuteOne = false;
			}

			if( m_ExecuteAll )
			{
				for( auto& rFunc : m_CommandBuffer )
					rFunc();

				m_CommandBuffer.clear();

				m_ExecuteAll = false;
			}
		}

		m_Running->store( false );
	}

}