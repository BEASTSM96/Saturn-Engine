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
        private readonly UserTarget TargetToBuild = null;

        public string[] Args;

        public Toolchain Toolchain { get; set; }

        private readonly FileCache FileCache = null;

        private bool HasCompiledAnyFile = false;

        private int NumTasksFailed = 0;

        private readonly List<List<string>> FilesPerThread = new List<List<string>>();
        private readonly List<bool> ThreadsCompleted = new List<bool>();

        private bool IsRebuild = false;

        List<string> SourceFiles = null;
        List<string> BuildFiles = null;

        // Args:
        // 0: The Action, BUILD, REBULD, CLEAN <--TODO
        // 1: The project name
        // 2: The target platform, Win64
        // 3: The configuration, Debug, Release, Dist
        // 4: The project location
        public Application(string[] args) 
        {
          //  Thread.Sleep(7000);

            Args = args;

            // Setup project info from args.
            ProjectInfo.Instance.Setup( Args );

            TargetToBuild = UserTarget.SetupUserTarget();

            if (TargetToBuild == null) 
            {
                Console.WriteLine( "Could not find a user target!, looking for {0} Please regenerate it in the engine!", ProjectInfo.Instance.BuildRuleFile );
            }

            switch (ProjectInfo.Instance.TargetPlatformKind) 
            {
                case ArchitectureKind.x86:
                case ArchitectureKind.x64:
                    {
                        Toolchain = new MSVCToolchain(TargetToBuild);
                    } break;

                default:
                    break;
            }

            FileCache = FileCache.Load();
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

        private void CompileSourceFiles() 
        {
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
                    {
                        count = 0;
                    }

                    List<string> filesForThread = new List<string>( SourceFiles.GetRange( start, count ) );
                    FilesPerThread.Add(filesForThread);
                    ThreadsCompleted.Add(false);

                    ThreadPool.QueueUserWorkItem(new WaitCallback(CompileFiles_ForThread), i);
                }

                while (ThreadsCompleted.Contains(false))
                {
                    // Wait
                    Thread.Sleep( 1 );
                }
            }
            else 
            {
                Console.WriteLine("Compiling single threaded.");

                // Pass all the files for the one thread.
                FilesPerThread.Add(SourceFiles);
                ThreadsCompleted.Add(false);

                CompileFiles_ForThread(0);
            }
        }

        private void CompileBuildFolderFiles() 
        {
            FilesPerThread.Clear();
            ThreadsCompleted.Clear();

            int threadCount = 0;
            threadCount = (int)Math.Ceiling((double)BuildFiles.Count / (Environment.ProcessorCount / 2));

            if (threadCount > 1)
            {
                Console.WriteLine(String.Format("Building with {0} threads", threadCount));

                ThreadPool.SetMaxThreads(threadCount, threadCount);

                // Divide the files into separate lists for each thread.
                for (int i = 0; i < threadCount; i++)
                {
                    int start = i * (BuildFiles.Count / threadCount);
                    int end = (i == threadCount - 1) ? BuildFiles.Count : i + 1 * (BuildFiles.Count / threadCount);
                    int count = end - start;

                    if (count < 0)
                    {
                        count = 0;
                    }

                    List<string> filesForThread = new List<string>(BuildFiles.GetRange(start, count));
                    FilesPerThread.Add(filesForThread);
                    ThreadsCompleted.Add(false);

                    ThreadPool.QueueUserWorkItem(new WaitCallback(CompileFiles_ForThread), i);
                }

                while (ThreadsCompleted.Contains(false))
                {
                    // Wait
                    Thread.Sleep(1);
                }
            }
            else
            {
                Console.WriteLine("Compiling single threaded.");

                // Pass all the files for the one thread.
                FilesPerThread.Add(BuildFiles);
                ThreadsCompleted.Add(false);

                CompileFiles_ForThread(0);
            }
        }

        private void SearchForFiles() 
        {
            SourceFiles = DirectoryTools.DirSearch(ProjectInfo.Instance.SourceDir, true);
            BuildFiles = DirectoryTools.DirSearch(ProjectInfo.Instance.BuildDir);

            // Remove the entry file if we are not an exe.
            if (ProjectInfo.Instance.CurrentConfigKind != ConfigKind.Dist)
            {
                string EntryFilepath = Path.Combine(ProjectInfo.Instance.BuildDir, string.Format("{0}.Entry.cpp", ProjectInfo.Instance.Name));

                BuildFiles.Remove(EntryFilepath);
            }
        }

        public void Run() 
        {
            Stopwatch time = Stopwatch.StartNew();

            Console.WriteLine("==== Saturn Build Tool v0.0.2 ====");

            IsRebuild = Args[0] == "/REBUILD";

            SearchForFiles();

            // Compile all source files and all "build folder" files next.
            CompileSourceFiles();
            CompileBuildFolderFiles();

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
