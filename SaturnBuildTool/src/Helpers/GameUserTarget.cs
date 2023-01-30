using BuildTool;

namespace BuildTool
{
    public class GameUserTarget : UserTarget
    {
        public override void Init() 
        {
            base.Init();

            // Always use x64
            Architectures = new[] { TargetKind.Win64 };

            BuildConfigs = new[] { ConfigKind.Debug, ConfigKind.Release, ConfigKind.Dist };

            // Games are DLLs in the editor
            // TODO: Allow for games to be an exe.
            OutputType = LinkerOutput.SharedLibrary;

            // Include source.
            Includes.Add( "src" );
        }
    }
}
