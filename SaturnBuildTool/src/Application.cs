using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using SaturnBuildTool.Cache;
using SaturnBuildTool.Tools;

namespace SaturnBuildTool
{
    internal class Application
    {
        private UserTarget TargetToBuild = null;

        public string SourceDir { get; set; }

        public string BuildDir { get; set; }

        public string ProjectDir { get; set; }

        public string[] Args;

        public Toolchain Toolchain { get; set; }

        private string CacheLocation = null;

        private FileCache FileCache = null;

        private void FindSourceDir()
        {
            SourceDir = Path.Combine(ProjectDir, "Scripts");

            // We know if we are the game that we source is in the same dir as the project.
            // However if it does not exist then we are the engine.
            if (!Directory.Exists(SourceDir))
            {
                SourceDir = Path.Combine(ProjectDir, Args[1]);

                SourceDir = Path.Combine(SourceDir, "src");

                SourceDir = Path.Combine(SourceDir, Args[1]);
            }

            SourceDir = SourceDir.Replace("/", "\\");
        }

        private void FindBuildFolder()
        {
            BuildDir = Path.Combine(ProjectDir, "Build");

            BuildDir = BuildDir.Replace("/", "\\");
        }

        private void FindProjectDir()
        {
            int index = Args[4].IndexOf('/');

            ProjectDir = Args[4].Substring(index + 1);
            ProjectDir = ProjectDir.Replace("/", "\\");
        }

        public Application(string[] args) 
        {
            Args = args;

            FindProjectDir();
            FindBuildFolder();
            FindSourceDir();

            Target.Instance.Init( Args[2] );

            BuildConfig.Instance.Init( Args[3] );

            BuildTargetFile.Instance.InitBuildFile(SourceDir, Args[1]);
            BuildTargetFile.Instance.CreateBuildFile();

            TargetToBuild = UserTarget.SetupUserTarget(BuildTargetFile.Instance.BuildFile);

            if(TargetToBuild == null)
                Console.WriteLine( "Could not find a user target!" );

            TargetToBuild.ProjectName = Args[1];
            TargetToBuild.ProjectName = TargetToBuild.ProjectName.Replace("/", string.Empty);

            switch (Target.Instance.GetTargetKind()) 
            {
                case TargetKind.Win86:
                case TargetKind.Win64:
                    {
                        Toolchain = new MSVCToolchain(TargetToBuild);
                    } break;
            }

            CacheLocation = ProjectDir + "\\filecache.fc";
            FileCache = FileCache.Load(CacheLocation);
        }

        public void Run() 
        {
            Stopwatch time = Stopwatch.StartNew();

            Console.WriteLine("==== Saturn Build Tool v0.0.1 ====");

            List<string> sourceFiles = DirectoryTools.DirSearch(SourceDir, true);
            List<string> sourceBuildFiles = DirectoryTools.DirSearch(BuildDir);

            bool HasCompiledAnyFile = false;
            int NumTaskFailed = 0;

            foreach (string file in sourceFiles)
            {
                // We are only building c++ files.
                if (!FileCache.IsCppFile(file))
                {
                    continue;
                }

                // Only compile the file if it has not be changed.
                FileCache.FilesInCache.TryGetValue(file, out DateTime LastTime);

                if (Args[0] == "/REBUILD")
                {
                    int exitCode = Toolchain.Compile(file);

                    if (exitCode == 0)
                    {
                        HasCompiledAnyFile = true;

                        if (!FileCache.IsFileInCache(file))
                            FileCache.CacheFile(file);
                    }
                    else
                        NumTaskFailed++;
                }
                else if (Args[0] == "/BUILD" && (LastTime != File.GetLastWriteTime(file)))
                {
                    int exitCode = Toolchain.Compile(file);

                    if (exitCode == 0)
                    {
                        HasCompiledAnyFile = true;

                        if (!FileCache.IsFileInCache(file))
                            FileCache.CacheFile(file);
                    }
                    else
                        NumTaskFailed++;
                }
            }

            if( BuildConfig.Instance.GetTargetConfig() >= ConfigKind.DistDebug )
            {
                foreach (string file in sourceBuildFiles)
                {
                    // We are only building c++ files.
                    if (!FileCache.IsCppFile(file))
                    {
                        continue;
                    }

                    int exitCode = Toolchain.Compile(file, false);

                    if (exitCode == 0)
                    {
                        HasCompiledAnyFile = true;
                    }
                    else
                        NumTaskFailed++;
                }
            }

            Console.WriteLine(string.Format("{0} task(s) failed.", NumTaskFailed));

            if (HasCompiledAnyFile && NumTaskFailed == 0)
                Toolchain.Link();

            if (HasCompiledAnyFile)
            {
                FileCache.RT_WriteCache(FileCache);
            }

            Console.WriteLine("Done building in {0}", time.Elapsed);
        }
    }
}
