using System;
using System.IO;
using System.Diagnostics;
using System.Collections.Generic;

using BuildTool.Tools;
using BuildTool.Toolchain;
using BuildTool.Cache;

namespace BuildTool
{
	class EntryPoint
	{
        static void Main(string[] Args)
		{
			Console.WriteLine("==== Saturn Build Tool v0.0.1 ====");

			// Find the src of the current project we are trying to build.
			string OurDir = Directory.GetCurrentDirectory();
			OurDir = Directory.GetParent( OurDir ).ToString();
			OurDir = Directory.GetParent( OurDir ).ToString();

			OurDir += Args[1];

			OurDir = OurDir.Replace( "/", "\\" );

			Target.Instance.TargetName = Args[2];

			Target.Instance.TargetName = Target.Instance.TargetName.Replace( "/", string.Empty );

			BuildConfig.Instance.ConfigName = Args[3];
			BuildConfig.Instance.ConfigName = BuildConfig.Instance.ConfigName.Replace( "/", string.Empty );

			//
			string BuildFile = OurDir + "\\src";
			BuildFile += Args[1];
			BuildFile += ".Build.cs";
			BuildFile = BuildFile.Replace("/", "\\");

			UserTarget target = UserTarget.SetupUserTarget(BuildFile);
			target.ProjectName = Args[1];

			//
			MVSCToolchain Toolchain = new MVSCToolchain(target);

			string cacheLocation = OurDir + "\\filecache.fc";
			FileCache fileCache = FileCache.Load( cacheLocation );

			List<string> sourceFiles = DirectoryTools.DirSearch( OurDir, true );

			bool HasCompiledAnyFile = false;

			int NumTaskFailed = 0;

			foreach ( string file in sourceFiles ) 
			{
				if (!fileCache.IsFileInCache(file))
					fileCache.CacheFile(file);

				// Only compile the file if it has not be changed.
				DateTime LastTime;
				fileCache.FilesInCache.TryGetValue(file, out LastTime);

                if( LastTime != File.GetLastWriteTime(file) )
                {
                    int exitCode = Toolchain.Compile(file);

                    if (exitCode == 0)
                        HasCompiledAnyFile = true;
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

			Console.WriteLine("Done building");
		}
    }
}
