using System;
using System.Reflection;
using System.Threading;

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
          //  Thread.Sleep( 7000 );

            if( IsHelpCommand( args ) ) { return; }

            if (args.Length <= 4)
            {
                Console.WriteLine("ERROR: You must provide 5 arguments!");
                return;
            }

            /*
            if (!ValidateArgs(args)) 
            {
                Console.WriteLine("ERROR: Validate args failed!");
                return;
            }
            */

            // Safe to continue
            Application app = new Application(args);
            app.Run();
        }

        static bool IsHelpCommand(string[] args) 
        {
            if( args.Length > 0 )
            {
                if ( args[0] == "/HELP" ) 
                {
                    Console.WriteLine("Saturn Build Tool v0.2.0");
                    Console.WriteLine("Valid usage:");
                    Console.WriteLine(" 1) Argument must be /BUILD, /REBUILD or /CLEAN");
                    Console.WriteLine(" 2) Argument must not contain spaces");
                    Console.WriteLine(" 3) Argument must either be /Win64 or /Win86 ");
                    Console.WriteLine(" 4) Argument must be /DEBUG, /RELEASE /DIST");
                    Console.WriteLine(" 5) Argument must path must exist");
                    Console.WriteLine(" 6) You must provide 5 arguments");
                    Console.WriteLine("Arguments");
                    Console.WriteLine(" 1) The Action to do for the project: /BUILD, /REBUILD or /CLEAN");
                    Console.WriteLine(" 2) The Project name");
                    Console.WriteLine(" 3) The Target platform: /Win64 or /Win86 ");
                    Console.WriteLine(" 4) The Target configuration: /DEBUG, /RELEASE /DIST");
                    Console.WriteLine(" 5) The Project location");

                    return true;
                }
            }

            return false;
        }

        static bool ValidateArgs(string[] args)
        {
            for(int i = 0; i < args.Length; i++) 
            {
                Console.WriteLine(args[i]);
            }

            if( args[0] != "/BUILD" || args[0] != "/REBUILD" || args[0] != "/CLEAN")
            {
                Console.WriteLine("ERROR: Action argument must be /BUILD /REBUILD or /CLEAN");
                Console.WriteLine( string.Format( "ERROR: Action was {0}", args[ 0 ] ) );
                return false;
            }

            if( args[1].Contains(" ") )
            {
                Console.WriteLine("ERROR: Project argument must not contain a space!");
                return false;
            }

            if( args[2] != "/Win64" || args[2] != "/Win86")
            {
                Console.WriteLine("ERROR: Target platform argument must be /Win64 or /Win86");
                return false;
            }

            if (args[3] != "/Debug" || args[3] != "/Release" || args[3] != "/Dist")
            {
                Console.WriteLine("ERROR: Configuration argument must be /Debug /Release or /Dist");
                return false;
            }

            return true;
        }
    }
}
