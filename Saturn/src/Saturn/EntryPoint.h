/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 BEAST                                                                  *
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

extern Saturn::Application* Saturn::CreateApplication();

int main( int argc, char** argv )
{
	//Saturn::Log::Init();

	Saturn::InitCore();

	auto agrvcx = *argv;

	SAT_CORE_INFO( "Exe : {0}", agrvcx );

	SAT_PROFILE_BEGIN_SESSION( "Startup", "SaturnProfile-Startup.json" );
	Saturn::Application* app = Saturn::CreateApplication();
	SAT_PROFILE_END_SESSION();

	SAT_PROFILE_BEGIN_SESSION( "Runtime", "SaturnProfile-Runtime.json" );
	app->Run();
	SAT_PROFILE_END_SESSION();

	SAT_PROFILE_BEGIN_SESSION( "Shutdown", "SaturnProfile-Shutdown.json" );
	delete app;
	SAT_PROFILE_END_SESSION();

	Saturn::EndCore();
}
