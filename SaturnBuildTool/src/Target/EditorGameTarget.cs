using System;
using System.IO;

namespace SaturnBuildTool.Target
{
    internal class EditorGameTarget : GameUserTarget
    {
        public override void Init() 
        {
            base.Init();

            BuildConfigs = new[] { ConfigKind.Debug, ConfigKind.Release, ConfigKind.Dist };

            // Games are DLLs in the editor.
            OutputType = LinkerOutput.SharedLibrary;
        }
    }
}
