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

// Game client main.
// DO NOT MODIFY

// Saturn client main:
extern int _main( int, char** );

int main()
{
	// Hand it off to Saturn:
	return _main( count, args );
}

#if defined ( _WIN32 )

int WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd ) 
{
	return main( __argc, __argv );
}

#endif // _WIN32


// Client default app.
class __GameApplication : public Saturn::Application
{
	__GameApplication(const Saturn::ApplicationSpecification& spec )
		: Saturn::Application( spec ) 
	{
	}

	virtual void OnInit() override 
	{
		m_RuntimeLayer = new Saturn::RuntimeLayer();
		PushLayer( m_RuntimeLayer );
	}

	virtual void OnShutdown() override
	{
		PopLayer( m_RuntimeLayer );
		delete m_RuntimeLayer;
	}
private:
	Saturn::RuntimeLayer* m_RuntimeLayer = nullptr;
}

Saturn::Application* Saturn::CreateApplication( int argc, char** argv ) 
{
	return new __GameApplication( {} );
}