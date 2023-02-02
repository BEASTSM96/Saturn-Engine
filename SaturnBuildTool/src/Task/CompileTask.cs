using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

using SaturnBuildTool.Toolchain;
using SaturnBuildTool.Tools;

namespace SaturnBuildTool
{
    internal class CompileTask : AsyncTask
    {
        private string InputFile;
        private UserTarget TargetToBuild;

        public CompileTask(string inputFile, UserTarget target)
        {
            InputFile = inputFile;
            TargetToBuild = target;
        }

        public override int Execute()
        {
            if( Path.GetExtension( InputFile ) != ".cpp" )
                return 0;

            string VSLocation = VSWhere.FindVSRootDir();

            var Args = new List<string>();

            ProcessStartInfo processStart = new ProcessStartInfo();
            processStart.CreateNoWindow = true;

            switch (Target.Instance.GetTargetKind())
            {
                case TargetKind.Win64:
                    {
                        processStart.FileName = VSLocation + "/VC/Tools/MSVC/14.32.31326/bin/Hostx64/x64/cl.exe";
                    }
                    break;

                case TargetKind.Win86:
                    {
                        processStart.FileName = VSLocation + "/VC/Tools/MSVC/14.32.31326/bin/Hostx64/x86/cl.exe";
                    }
                    break;
            }

            processStart.CreateNoWindow = true;
            processStart.RedirectStandardOutput = true;
            processStart.RedirectStandardError = true;
            processStart.UseShellExecute = false;

            Process clProcess = new Process();
            clProcess.StartInfo = processStart;

            // Parse Args
            Args.Add(" /nologo");

            // Compile
            Args.Add(" /c");
            
            Args.Add(" /errorreport:prompt");
            Args.Add(" /verbosity:quiet");
            
            // Debugging.
            Args.Add(" /ZI");

            // Compile for C++
            Args.Add(" /std:c++latest /D _HAS_EXCEPTIONS=0");

            // Unwind semantics.
            Args.Add(" /EHsc");

            if(TargetToBuild.CurrentConfig == ConfigKind.Debug)
                Args.Add(" /MTd" );
            else
                Args.Add(" /MT" );

            // Out
            string outFile = string.Format("{0}\\{1}", TargetToBuild.OutputPath, Path.GetFileName(Path.ChangeExtension(InputFile, ".obj")));

            Args.Add(string.Format(" /Fo\"{0}\"", outFile));

            // Marcos
            List<string> marcos = TargetToBuild.PreprocessorDefines;

            foreach (string name in marcos)
            {
                Args.Add(string.Format(" /D\"{0}\"", name));
            }

            // Includes
            List<string> incs = TargetToBuild.Includes;
            foreach (string include in incs)
            {
                Args.Add(string.Format(" /I\"{0}\"", include));
            }

            Args.Add(string.Format(" /I\"{0}\"", VSLocation + "/VC/Tools/MSVC/14.32.31326/include"));

            // Windows SDK
            string includeSDKFolder = WindowsSDK.GetIncludePaths();
            Args.Add(string.Format(" /I\"{0}\"", includeSDKFolder + "/ucrt"));
            Args.Add(string.Format(" /I\"{0}\"", includeSDKFolder + "/um"));
            Args.Add(string.Format(" /I\"{0}\"", includeSDKFolder + "/shared"));

            // In
            Args.Add(string.Format(" /Tp\"{0}\"", InputFile));

            // Start the compile
            processStart.Arguments = string.Join("", Args);

            Console.WriteLine("Building " + Path.GetFileName(InputFile));

            clProcess.EnableRaisingEvents = true;

            clProcess.Start();

            clProcess.WaitForExit();

            // Write error output
            Console.WriteLine(clProcess.StandardOutput.ReadToEnd().Trim());

            return clProcess.ExitCode;
        }
    }
}
