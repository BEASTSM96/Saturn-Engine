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

#include <Saturn/Vulkan/Texture.h>
#include <Saturn/Core/Layer.h>

namespace Saturn {
	
	class TitleBar;

	struct ProjectInformation
	{
		std::string Name;
		std::filesystem::path Filepath;
		std::filesystem::path AssetPath;

		std::string LastWriteTime;
	
		uint64_t Version = SAT_CURRENT_VERSION;
	};

	class ProjectBrowserLayer : public Layer
	{
	public:
		ProjectBrowserLayer();
		~ProjectBrowserLayer();

		void OnUpdate( Timestep time ) override;
		void OnImGuiRender() override;
		void OnEvent( RubyEvent& rEvent ) override;
		void OnAttach() override;
		void OnDetach() override;

	private:
		bool OnKeyPressed( RubyKeyEvent& rEvent );

		void OpenProject( const ProjectInformation& rProject );
		void CreateProject( const std::filesystem::path& rPath );
		void DrawRecentProject( const ProjectInformation& rProject );

	private:
		Ref<Texture2D> m_NoIconTexture = nullptr;

		TitleBar* m_TitleBar = nullptr;

		char* m_SaturnDirBuffer = new char[ 1024 ];
		std::filesystem::path m_SaturnDir;

		char* m_ProjectNameBuffer = new char[ 1024 ];
		char* m_ProjectFilePathBuffer = new char[ 1024 ];

		bool m_ShowNewProjectPopup = false;
		bool m_ShouldThreadTerminate = false;

		std::vector<ProjectInformation> m_RecentProjects;
		std::thread m_RecentProjectThread;

		bool m_HasSaturnDir = false;

	};
}