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

#include <Saturn.h>
#include <Saturn/EntryPoint.h>

#include "EditorLayer.h"

class EditorApplication : public Saturn::Application
{
public:
	EditorApplication( Saturn::ApplicationCommandLineArgs args, const Saturn::ApplicationProps& props ) : Application( args, props )
	{
		m_EditorLayer = new Saturn::EditorLayer();
		PushOverlay( m_EditorLayer );

		//TODO: Make a better icon as it does not fit
		//m_Window->SetWindowImage( "assets/.github/i/sat/SaturnLogov1.png" );
	}

	Saturn::EditorLayer& GetEditorLayer() { return *m_EditorLayer; }
	const Saturn::EditorLayer& GetEditorLayer() const { return *m_EditorLayer; }

private:
	Saturn::EditorLayer* m_EditorLayer;
};

Saturn::Application* Saturn::CreateApplication( Saturn::ApplicationCommandLineArgs args )
{
	return new EditorApplication( args, { "SaturnEditor (Editor Pre Init), (*UUID not yet loaded, *branch)", 1600, 900 } );
}
