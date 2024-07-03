using System;

namespace SaturnBuildTool
{
    // Valid arguments for SCLASS macro
    [Flags]
    public enum SC
    {
        None = 0,
        Spawnable = 1 << 0,
        VisibleInEditor = 1 << 1,
        NoMetadata = 1 << 2,
    }
}
