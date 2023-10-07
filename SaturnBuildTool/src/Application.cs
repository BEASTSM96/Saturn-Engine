using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
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

        private bool HasCompiledAnyFile = false;

        private int NumTasksFailed = 0;

        private List<List<string>> FilesPerThread = new List<List<string>>();
        private List<bool> ThreadsCompleted = new List<bool>();

        private bool IsRebuild = false;

        List<string> SourceFiles = null;
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

            if (TargetToBuild == null) 
            {
                Console.WriteLine( "Could not find a user target!, looking for {0} Please regenerate it in the engine!", BuildTargetFile.Instance.BuildFile );
            }

            TargetToBuild.ProjectName = Args[1];
            TargetToBuild.ProjectName = TargetToBuild.ProjectName.Replace("/", string.Empty);

            switch (Target.Instance.GetTargetKind()) 
            {
                case TargetKind.Win86:
                case TargetKind.Win64:
                    {
                        Toolchain = new MSVCToolchain(TargetToBuild);
                    } break;

                default:
                    break;
            }

            CacheLocation = ProjectDir + "\\filecache.fc";
            FileCache = FileCache.Load(CacheLocation);
        }

        private void CompileFiles_ForThread(object index) 
        {
            int ThreadIndex = (int)index;
            List<string> Files;

            lock (new object())
            {
                Files = FilesPerThread[ThreadIndex];
            }

            foreach (string file in Files)
            {
                // We are only building c++ files.
                if (!FileCache.IsCppFile(file))
                {
                    continue;
                }

                // Only compile the file if it has not be changed.
                FileCache.FilesInCache.TryGetValue(file, out DateTime LastTime);

                if (IsRebuild)
                {
                    int exitCode = Toolchain.Compile(file);

                    if (exitCode == 0)
                    {
                        HasCompiledAnyFile = true;

                        if (!FileCache.IsFileInCache(file)) 
                        {
                            FileCache.CacheFile(file);
                        }
                    }
                    else
                        NumTasksFailed++;
                }
                else if (LastTime != File.GetLastWriteTime(file))
                {
                    int exitCode = Toolchain.Compile(file);

                    if (exitCode == 0)
                    {
                        HasCompiledAnyFile = true;

                        if (!FileCache.IsFileInCache(file)) 
                        {
                            FileCache.CacheFile(file);
                        }
                    }
                    else
                        NumTasksFailed++;
                }
            }

            ThreadsCompleted[ThreadIndex] = true;
        }

        private bool CompileFiles() 
        {
            SourceFiles = DirectoryTools.DirSearch(SourceDir, true);
            
            int threadCount = 0;
            threadCount = (int)Math.Ceiling((double)SourceFiles.Count / (Environment.ProcessorCount / 2));

            if (threadCount > 1)
            {
                Console.WriteLine( String.Format( "Building with {0} threads", threadCount ) );

                ThreadPool.SetMaxThreads(threadCount, threadCount);

                // Divide the files into separate lists for each thread.
                for (int i = 0; i < threadCount; i++)
                {
                    int start = i * (SourceFiles.Count / threadCount);
                    int end = (i == threadCount - 1) ? SourceFiles.Count : i + 1 * (SourceFiles.Count / threadCount);
                    int count = end - start;

                    if (count < 0)
                        count = 0;

                    Console.WriteLine(end - start);

                    List<string> filesForThread = new List<string>( SourceFiles.GetRange( start, count ) );
                    FilesPerThread.Add(filesForThread);
                    ThreadsCompleted.Add(false);

                    ThreadPool.QueueUserWorkItem(new WaitCallback(CompileFiles_ForThread), i);
                }

                while (ThreadsCompleted.Contains(false))
                {
                    // Wait
                }
            }
            else 
            {
                Console.WriteLine("Compiling single threaded.");
                CompileFiles_ForThread(null);
            }

            return NumTasksFailed == 0;
        }

        public void Run() 
        {
            Stopwatch time = Stopwatch.StartNew();

            Console.WriteLine("==== Saturn Build Tool v0.0.1 ====");

            IsRebuild = Args[0] == "/REBUILD";

            List<string> sourceBuildFiles = DirectoryTools.DirSearch(BuildDir);
            CompileFiles();

            if( BuildConfig.Instance.GetTargetConfig() >= ConfigKind.DistDebug )
            {
                foreach (string file in sourceBuildFiles)
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
                        int exitCode = Toolchain.Compile(file, false);

                        if (exitCode == 0)
                        {
                            HasCompiledAnyFile = true;

                            if (!FileCache.IsFileInCache(file))
                                FileCache.CacheFile(file);
                        }
                        else
                            NumTasksFailed++;
                    }
                    else if (Args[0] == "/BUILD" && (LastTime != File.GetLastWriteTime(file)))
                    {
                        int exitCode = Toolchain.Compile(file, false);

                        if (exitCode == 0)
                        {
                            HasCompiledAnyFile = true;

                            if (!FileCache.IsFileInCache(file))
                                FileCache.CacheFile(file);
                        }
                        else
                            NumTasksFailed++;
                    }
                }
            }

            Console.WriteLine(string.Format("{0} task(s) failed.", NumTasksFailed));

            if (HasCompiledAnyFile && NumTasksFailed == 0)
                Toolchain.Link();

            if (HasCompiledAnyFile)
            {
                FileCache.RT_WriteCache(FileCache);
            }

            Console.WriteLine("Done building in {0}", time.Elapsed);
        }
    }
}
