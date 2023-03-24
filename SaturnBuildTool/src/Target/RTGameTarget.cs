using System;

namespace SaturnBuildTool
{
    // Usage for Dist-Debug, Dist-Release and Dist-Full.
    public class RTGameTarget : GameUserTarget
    {
        public override void Init()
        {
            base.Init();

            BuildConfigs = new[] { ConfigKind.DistDebug, ConfigKind.DistRelease, ConfigKind.DistFull };

            // Games are exes outside of the editor.
            OutputType = LinkerOutput.Executable;
        }
    }
}
