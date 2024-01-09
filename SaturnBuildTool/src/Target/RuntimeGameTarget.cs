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

            // Remove Tracy because this will never be used in Dist
            // And is only to be used in Debug and Release
            Links.Remove( "Tracy.lib" );
            // TODO: Remove assimp
            // TODO: Make shared storage a static library because we are no longer sharing memory between the Game DLL and the Editor Exe.
        }
    }
}
