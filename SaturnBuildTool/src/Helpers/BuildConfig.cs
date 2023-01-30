﻿using System;

namespace BuildTool
{
    public enum ConfigKind
    {
        Debug,
        Release,
        Dist, // Saturn Only
    }

    internal class BuildConfig
    {
        public static readonly BuildConfig Instance = new BuildConfig();

        public string ConfigName { get; set; }

        public ConfigKind GetTargetConfig()
        {
            if (ConfigName == "Debug")
                return ConfigKind.Debug;
            else if (ConfigName == "Release")
                return ConfigKind.Release;
            else // Assume is Dist
                return ConfigKind.Dist;
        }

        public BuildConfig() { }

        public BuildConfig(string target)
        {
            ConfigName = target;
        }
    }
}
