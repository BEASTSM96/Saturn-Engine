#include "sppch.h"
#include "Base.h"

#include "Saturn/Log.h"

namespace Saturn {

	void InitCore( void )
	{
		Saturn::Log::Init();

		SAT_CORE_TRACE("============= SATURN ENGINE=============");

		const char* ltext = "\n MIT License\n Copyright(c) 2020 BEAST \n Permission is hereby granted, free of charge, to any person obtaining a copy of this softwareand associated documentation files(the 'Software'), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and /or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions : \n The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software. \n THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.";

		SAT_CORE_TRACE("{0}", ltext);
		SAT_CORE_TRACE("============= v0.a01=============");
		SAT_CORE_TRACE("{0} ({1})", "Welcome to", "v0.a01");
		SAT_CORE_TRACE("API Docs (https://beastsm96.github.io/Projects/Saturn-Engine/api/v0.a01)");
	}

	void EndCore( void )
	{
		SAT_CORE_WARN("[v0.a01] Shuting Down!");
	}

}