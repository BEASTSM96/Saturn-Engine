using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SaturnBuildTool.Tools
{
    internal static class BuildTargetFile
    {
        public static string GetBuildFile( string SourceDir, string TargetName )
        {
            ConfigKind config = BuildConfig.Instance.GetTargetConfig();

            switch (config) 
            {
                case ConfigKind.Debug:
                case ConfigKind.Release:
                case ConfigKind.Dist: 
                    {
                        string BuildFile = SourceDir;
                        BuildFile += TargetName;
                        BuildFile += ".Build.cs";
                        BuildFile = BuildFile.Replace("/", "\\");

                        return BuildFile;
                    }

                case ConfigKind.DistDebug:
                case ConfigKind.DistRelease:
                case ConfigKind.DistFull:
                    {
                        string BuildFile = SourceDir;
                        BuildFile += TargetName;
                        BuildFile += ".RT_Build.cs";
                        BuildFile = BuildFile.Replace("/", "\\");

                        return BuildFile;
                    }
            }

            return "";
        }

    }
}
