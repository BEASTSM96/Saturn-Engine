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

#include "Saturn/Core/Base.h"
#include "Saturn/Asset/Asset.h"

#include "Saturn/NodeEditor/UI/NodeEditor.h"

namespace Saturn {

	class NodeCacheSettings
	{
	public:
		static inline NodeCacheSettings& Get() { return *SingletonStorage::GetOrCreateSingleton<NodeCacheSettings>(); }
	public:
		[[nodiscard]] bool WriteEditorSettings( Ref<NodeEditor>& rNodeEditor );

	private:
		Buffer TryReadFileForWriting( const std::filesystem::path& rFilepath );

		void OverrideFile( const std::filesystem::path& rFilepath, Ref<NodeEditor>& rNodeEditor );
		void AppendFile( Buffer& rBuffer, const std::filesystem::path& rFilepath, Ref<NodeEditor>& rNodeEditor );
	};

	class NodeCacheEditor
	{
	public:
	private:
	};
	
}