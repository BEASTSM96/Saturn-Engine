using System;
using System.IO;
using System.Diagnostics;
using System.Collections.Generic;

using SaturnBuildTool.Tools;
using SaturnBuildTool.Toolchain;
using SaturnBuildTool.Cache;

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
		static void Main(string[] Args)
		{
			Stopwatch time = Stopwatch.StartNew();

			Console.WriteLine("==== Saturn Build Tool v0.0.1 ====");

			int index = Args[4].IndexOf('/');

			string ProjectDir = Args[4].Substring( index + 1 );

			ProjectDir = ProjectDir.Replace("/", "\\");

			string SourceDir = Path.Combine(ProjectDir, "Scripts");

			// We know if we are the game that we source is in the same dir as the project.
			// However if it does not exist then we are the engine.
			if ( !Directory.Exists( SourceDir ) ) 
			{
				SourceDir = Path.Combine( ProjectDir, Args[1] );

				SourceDir = Path.Combine( SourceDir, "src" );

				SourceDir = Path.Combine( SourceDir, Args[1] );
			}

			SourceDir = SourceDir.Replace("/", "\\");

			Target.Instance.TargetName = Args[2];

			Target.Instance.TargetName = Target.Instance.TargetName.Replace( "/", string.Empty );

			BuildConfig.Instance.ConfigName = Args[3];
			BuildConfig.Instance.ConfigName = BuildConfig.Instance.ConfigName.Replace( "/", string.Empty );

			// Build.cs file
			string BuildFile = SourceDir;
			BuildFile += Args[1];
			BuildFile += ".Build.cs";
			BuildFile = BuildFile.Replace("/", "\\");

			UserTarget target = UserTarget.SetupUserTarget(BuildFile);
			target.ProjectName = Args[1];
			target.ProjectName = target.ProjectName.Replace( "/", string.Empty );

			// MSVC toolchain
			MVSCToolchain Toolchain = new MVSCToolchain(target);

			string cacheLocation = ProjectDir + "\\filecache.fc";
			FileCache fileCache = FileCache.Load( cacheLocation );

			List<string> sourceFiles = DirectoryTools.DirSearch( SourceDir, true );

			bool HasCompiledAnyFile = false;
			int NumTaskFailed = 0;

			foreach ( string file in sourceFiles ) 
			{
				// We are only building c++ files.
				if (!fileCache.IsCppFile(file))
					continue;

				// Only compile the file if it has not be changed.
				DateTime LastTime;
				fileCache.FilesInCache.TryGetValue(file, out LastTime);

				if (Args[0] == "/REBUILD")
				{
					int exitCode = Toolchain.Compile(file);

					if (exitCode == 0) 
					{
						HasCompiledAnyFile = true;

                        if (!fileCache.IsFileInCache(file))
                            fileCache.CacheFile(file);
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

                        if (!fileCache.IsFileInCache(file))
                            fileCache.CacheFile(file);
                    }
                    else
                        NumTaskFailed++;
                }
			}

			Console.WriteLine( string.Format( "{0} task(s) failed.", NumTaskFailed ) );

			if (HasCompiledAnyFile && NumTaskFailed == 0)
				Toolchain.Link();

			if (HasCompiledAnyFile) 
			{
				FileCache.RT_WriteCache( fileCache );
			}

			Console.WriteLine("Done building in {0}", time.Elapsed);
		}
    }
}
