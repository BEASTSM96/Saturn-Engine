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

#include "Saturn/Core/Base.h"
#include "Saturn/Log.h"

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

// bytes to KB
#define DIV 1024

#define WIDTH 7

#define SAT_TEST_MEMORY ::Saturn::Debuging::GetMemoryAliv()
#define SAT_TEST_MEMORY_OBJECT ::Saturn::Debuging::TestMemory()



namespace Saturn {

	namespace Debuging {

		/* Gets the Memory in GB
		*
		*
		*
		*
		*	Sample output:
		*	There is       45 percent of memory in use.
		*	There are		2029968 total KB of physical memory.
		*	There are		 00987388 free  KB of physical memory.
		*	There are		3884620 total KB of paging file.
		*	There are		2799776 free  KB of paging file.
		*	There are		2097024 total KB of virtual memory.
		*	There are		2084876 free  KB of virtual memory.
		*
		*
		*/
		void GetMemoryAliv() {
			MEMORYSTATUSEX statex;

			statex.dwLength = sizeof(statex);

			GlobalMemoryStatusEx(&statex);

			SAT_CORE_INFO("There is % {0} percent of memory in use.", statex.dwMemoryLoad);

			_tprintf(TEXT("There are %*I64d total KB of physical memory.\n"),
				WIDTH, statex.ullTotalPhys / DIV);
			_tprintf(TEXT("There are %*I64d free  KB of physical memory.\n"),
				WIDTH, statex.ullAvailPhys / DIV);
			_tprintf(TEXT("There are %*I64d total KB of paging file.\n"),
				WIDTH, statex.ullTotalPageFile / DIV);
			_tprintf(TEXT("There are %*I64d free  KB of paging file.\n"),
				WIDTH, statex.ullAvailPageFile / DIV);
			_tprintf(TEXT("There are %*I64d total KB of virtual memory.\n"),
				WIDTH, statex.ullTotalVirtual / DIV);
			_tprintf(TEXT("There are %*I64d free  KB of virtual memory.\n"),
				WIDTH, statex.ullAvailVirtual / DIV);
		}

		/* Adds the objects and will return null  */
		template<typename T>
		T* TestMemory(T* c, int a, int max) {
			for (size_t i = 0; i < a; i++)
			{
				int index;
				T* x = new T();
				index++;
			}

			SAT_CORE_INFO("Test Complete!, {0} Objects where made (note that this can be false)", a);
			GetMemoryAliv();


			return nullptr;
		}

		/* Adds the objects and will return null, but will delete it.  */
		template<typename T>
		T* TestMemory2(T* c, int a, int max) {
			for (size_t i = 0; i < a; i++)
			{
				int index;
				T* x = new T();
				index++;
			}

			SAT_CORE_INFO("Test Complete!, {0} Objects where made (note that this can be false)", a);
			GetMemoryAliv();

			return nullptr;

		}
	}
}
