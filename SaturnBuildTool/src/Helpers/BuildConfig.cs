using System;

namespace SaturnBuildTool
{
    public enum ConfigKind
    {
        Debug,
        Release,
        Dist,
        DistDebug,
        DistRelease,
        DistFull,
    }

    internal class BuildConfig
    {
        public static readonly BuildConfig Instance = new BuildConfig();

        public string ConfigName { get; set; }

        public ConfigKind GetTargetConfig()
        {
            if (ConfigName == "Debug")
            {
                return ConfigKind.Debug;
            }
            else if (ConfigName == "Release")
            {
                return ConfigKind.Release;
            }
            else if (ConfigName == "Dist")
            {
                return ConfigKind.Dist;
            }
            else if (ConfigName == "DDebug")
            {
                return ConfigKind.DistDebug;
            }
            else if (ConfigName == "DRelease")
            {
                return ConfigKind.DistRelease;
            }
            else if (ConfigName == "DF")
            {
                return ConfigKind.DistFull;
            }
            else 
                return ConfigKind.Dist;
        }

        public void Init(string config) 
        {
            ConfigName = config;
            ConfigName = ConfigName.Replace("/", string.Empty);
        }

        public BuildConfig() { }
    }
}
