using System;
using System.Collections.Generic;
using System.IO;


namespace SaturnBuildTool.Tools
{
    internal class BuildTargetFile
    {
        public static readonly BuildTargetFile Instance = new BuildTargetFile();

        public string BuildFile { get; set; }

        public void InitBuildFile(string SourceDir, string TargetName)
        {
            ConfigKind config = BuildConfig.Instance.GetTargetConfig();

            switch (config)
            {
                case ConfigKind.Debug:
                case ConfigKind.Release:
                case ConfigKind.Dist:
                    {
                        BuildFile = SourceDir;
                        BuildFile += TargetName;
                        BuildFile += ".Build.cs";
                        BuildFile = BuildFile.Replace("/", "\\");
                    } break;

                case ConfigKind.DistDebug:
                case ConfigKind.DistRelease:
                case ConfigKind.DistFull:
                    {
                        BuildFile = SourceDir;
                        BuildFile += TargetName;
                        BuildFile += ".RT_Build.cs";
                        BuildFile = BuildFile.Replace("/", "\\");
                    } break;
            }
        }

        public void CreateBuildFile()
        {
            FileStream fs = null;

            if (!File.Exists(BuildFile))
                fs = File.Create(BuildFile);

            if(fs != null)
                fs.Close();
        }
    }
}
