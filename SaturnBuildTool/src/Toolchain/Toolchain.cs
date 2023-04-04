using System;

namespace SaturnBuildTool
{
    internal class Toolchain
    {
        protected UserTarget TargetToBuild;

        public virtual int Compile(string InputFile, bool RunHeaderTool = true) { return 0; }
        public virtual int Link() { return 0; }
    }
}
