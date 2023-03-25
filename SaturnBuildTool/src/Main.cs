using System;

namespace SaturnBuildTool
{
	class EntryPoint
	{
        // Args:
        // 0: The Action, BUILD, REBULD, CLEAN. TODO
        // 1: The project name
        // 2: The target, Win64
        // 3: The configuration, Debug, Release, Dist
        // 4: The project location
        static void Main(string[] args)
		{
			Application app = new Application(args);

			app.Run();

			app = null;
		}
    }
}
