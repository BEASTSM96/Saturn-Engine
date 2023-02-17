using System;

namespace SaturnBuildTool
{
    // Vaild arguments for SPROPERTY macro
    [Flags]
    public enum SP
    {
        None = 0,
        VisibleInEditor = 1 << 0
    }
}
