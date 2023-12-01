using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SaturnBuildTool
{
    public enum ArchitectureKind
    {
        Win64,
        Win86,
        Unknown
    }

    public enum ConfigKind
    {
        Debug,
        Release,
        Dist,
        DistDebug,
        DistRelease,
        DistFull
    }

    // This class holds all of our needed information about the current project that is being built.
    // There should only ever be one of these created in the Build Tool.
    internal class ProjectInfo
    {
        public static readonly ProjectInfo Instance = new ProjectInfo();

        public string Name { get; set; }

        public string RootDirectory { get; set; }

        public string SourceDir { get; set; }

        // The directory where the "Build" folder are located.
        public string BuildDir { get; set; }

        // The directory where the ".Build.cs" files are located.
        public string TargetDir { get; set; }

        private string TargetPlatformName { get; set; }
        public ArchitectureKind TargetPlatformKind;

        private string ConfigName { get; set; }
        public ConfigKind CurrentConfigKind;

        public string FileCacheLocation { get; set; }

        // The current "*.Build.cs" file path.
        public string BuildRuleFile { get; set; }

        // Setup the ProjectInfo
        // Args:
        // 0: The Action, BUILD, REBULD, CLEAN. TODO
        // 1: The project name
        // 2: The target platform, Win64
        // 3: The configuration, Debug, Release, Dist
        // 4: The project location
        public void Setup( string[] Args ) 
        {
            string RootDir = Args[4];

            // Get the project directory.
            int index = RootDir.IndexOf( "/" );
            RootDirectory = RootDir.Substring( index + 1 );
            RootDirectory = RootDirectory.Replace( "/", "\\" );

            // Name
            Name = Args[1];

            // Source
            SourceDir = Path.Combine( RootDirectory, "Source" );
            SourceDir = Path.Combine( SourceDir, Name );
            SourceDir = SourceDir.Replace( "/", "\\" );

            // Target config file
            // So if we had a project it would be:
            // RootDirectory = C:\Projects\Example\
            // TargetDir = C:\Projects\Example\Source
            // As the source folder contains a folder with all of the source of the project
            // And then a file with the build rules.
            TargetDir = Path.Combine( RootDirectory, "Source" );
            TargetDir = TargetDir.Replace("/", "\\");

            // Build folder
            BuildDir = Path.Combine( RootDirectory, "Build" );
            BuildDir = BuildDir.Replace( "/", "\\" );

            // Filecache
            FileCacheLocation = RootDir + "\\filecahce.fc";

            TargetPlatformName = Args[2];
            StringToTargetPlatform();

            ConfigName = Args[3];
            StringToConfigKind();

            FindBuildRuleFile();
        }

        private void StringToConfigKind() 
        {
            if (ConfigName == "Debug")
            {
                CurrentConfigKind = ConfigKind.Debug;
            }
            else if (ConfigName == "Release")
            {
                CurrentConfigKind = ConfigKind.Release;
            }
            else if (ConfigName == "Dist")
            {
                CurrentConfigKind = ConfigKind.Dist;
            }
            else if (ConfigName == "DDebug")
            {
                CurrentConfigKind = ConfigKind.DistDebug;
            }
            else if (ConfigName == "DRelease")
            {
                CurrentConfigKind = ConfigKind.DistRelease;
            }
            else if (ConfigName == "DF")
            {
                CurrentConfigKind = ConfigKind.DistFull;
            }
            else
                CurrentConfigKind = ConfigKind.Dist;
        }

        private void StringToTargetPlatform() 
        {
            if (TargetPlatformName == "Win64")
                TargetPlatformKind = ArchitectureKind.Win64;
            else if (TargetPlatformName == "Win86")
                TargetPlatformKind = ArchitectureKind.Win86;

            TargetPlatformKind = ArchitectureKind.Unknown;
        }

        private void FindBuildRuleFile() 
        {
            switch (CurrentConfigKind)
            {
                case ConfigKind.Debug:
                case ConfigKind.Release:
                case ConfigKind.Dist:
                    {
                        BuildRuleFile = SourceDir;
                        BuildRuleFile += string.Format( "{0}.Build.cs", Name );

                        BuildRuleFile = BuildRuleFile.Replace( "\\", "/" );
                    } break;

                case ConfigKind.DistDebug:
                case ConfigKind.DistRelease:
                case ConfigKind.DistFull:
                    {
                        BuildRuleFile = SourceDir;
                        BuildRuleFile += string.Format("{0}.RT_Build.cs", Name);

                        BuildRuleFile = BuildRuleFile.Replace("\\", "/");
                    }
                    break;
            }
        }
    }
}
