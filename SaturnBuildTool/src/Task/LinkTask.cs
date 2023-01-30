using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

using BuildTool.Toolchain;
using BuildTool.Tools;

namespace BuildTool
{
    public enum LinkerOutput
    {
        StaticLibrary,
        SharedLibrary,
        Executable
    }

    internal class LinkTask : AsyncTask
    {
        private UserTarget TargetToBuild;

        public LinkTask( UserTarget target )
        {
            TargetToBuild = target;
        }

        public override int Execute()
        {
            var Args = new List<string>();
            string VSLocation = VSWhere.FindVSRootDir();

            ProcessStartInfo processStart = new ProcessStartInfo();
            processStart.CreateNoWindow = true;

            switch (Target.Instance.GetTargetKind())
            {
                case TargetKind.Win64:
                    {
                        processStart.FileName = VSLocation + "/VC/Tools/MSVC/14.32.31326/bin/Hostx64/x64/link.exe";
                    }
                    break;

                case TargetKind.Win86:
                    {
                        processStart.FileName = VSLocation + "/VC/Tools/MSVC/14.32.31326/bin/Hostx64/x86/link.exe";
                    }
                    break;
            }
            
            processStart.CreateNoWindow = false;
            processStart.RedirectStandardOutput = true;
            processStart.RedirectStandardError = true;
            processStart.UseShellExecute = false;

            Process clProcess = new Process();
            clProcess.StartInfo = processStart;

            Args.Add(" /NOLOGO");

            Args.Add(string.Format(" /SUBSYSTEM:CONSOLE /OUT:\"{0}\"", TargetToBuild.GetBinDir() ) );

            // std libraries
            string sdkLibPath = WindowsSDK.GetLibraryPaths();
            string msvcLibPath = GetMSVCLibraryPath();

            // MSVC
            Args.Add(string.Format(" /LIBPATH:\"{0}\"", msvcLibPath));

            // SDK
            Args.Add(string.Format(" /LIBPATH:\"{0}\"", sdkLibPath + "/um" + "/x64"));
            Args.Add(string.Format(" /LIBPATH:\"{0}\"", sdkLibPath + "/ucrt" + "/x64"));

            // Object files
            foreach (string file in TargetToBuild.GetIntermediateFiles())
            {
                Args.Add(string.Format(" \"{0}\"", file));
            }

            Args.Add(" /INCREMENTAL");
            Args.Add(" /MACHINE:x64");
            Args.Add(" /DEBUG:FULL" );

            string ilkPath = Path.Combine( TargetToBuild.OutputPath, TargetToBuild.ProjectName );
            ilkPath = Path.ChangeExtension( ilkPath, ".ilk" );

            Args.Add( string.Format( " /ILK:\"{0}\"", ilkPath ) );

            // Start the link...
            Console.WriteLine("Linking");

            clProcess.EnableRaisingEvents = true;

            processStart.Arguments = string.Join("", Args);
            clProcess.Start();

            clProcess.WaitForExit();

            Console.WriteLine( clProcess.StandardOutput.ReadToEnd().Trim() );

            return clProcess.ExitCode;
        }

        private string GetMSVCLibraryPath()
        {
            string VSLocation = VSWhere.FindVSRootDir();

            switch (Target.Instance.GetTargetKind())
            {
                case TargetKind.Win64:
                    {
                        VSLocation += "/VC/Tools/MSVC/14.32.31326/lib/x64";
                    }
                    break;

                case TargetKind.Win86:
                    {
                        VSLocation += "/VC/Tools/MSVC/14.32.31326/lib/x64";
                    }
                    break;
            }

            return VSLocation;
        }
    }
}
