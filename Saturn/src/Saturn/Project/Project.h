/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
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
#include "Saturn/Core/UUID.h"
#include <string>

namespace Saturn {

	class Project : public RefCounted
	{
	public:
		Project( std::string& filepath, std::string name );
		Project( UUID& uuid );
		virtual ~Project();

		/* Copies the assets from the working dir into the project dir */
		void CopyAssets();

	public:
		std::string& GetAssetsFolderPath() { return m_AssetsPath; }
		const std::string& GetAssetsFolderPath() const { return m_AssetsPath; }

		std::string& GetName() { return m_Name; }
		const std::string& GetName() const { return m_Name; }

		UUID& GetUUID() { return m_UUID; }
		const UUID& GetUUID() const { return m_UUID; }

	protected:
	private:
		// working dir is the Solution working dir
		std::string m_AssetsPath;
		std::string m_WorkingDir;
		std::string m_Name;
		UUID m_UUID;
	};

}