using System;

namespace SaturnBuildTool
{
    // Usage for Dist-Debug, Dist-Release and Dist-Full.
    public class RuntimeGameTarget : GameUserTarget
    {
        public override void Init()
        {
            base.Init();

            BuildConfigs = new[] { ConfigKind.Dist };

            // Games are exes outside of the editor.
            OutputType = LinkerOutput.Executable;
        }
    }
}
