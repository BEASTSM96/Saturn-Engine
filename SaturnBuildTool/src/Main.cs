using System.Reflection;

[assembly: AssemblyVersion("0.0.2.0")]
[assembly: AssemblyCompany("Saturn Engine")]
[assembly: AssemblyCopyright("2020 - 2023")]
[assembly: AssemblyProduct("Saturn Build Tool")]
[assembly: AssemblyDescription("Saturn Build Tool used for compiling games.")]

namespace SaturnBuildTool
{
	class EntryPoint
	{
        // Args:
        // 0: The Action, BUILD, REBULD, CLEAN. TODO
        // 1: The project name
        // 2: The target platform, Win64
        // 3: The configuration, Debug, Release, Dist
        // 4: The project location
        static void Main(string[] args)
		{
			Application app = new Application(args);

			app.Run();
        }
    }
}
