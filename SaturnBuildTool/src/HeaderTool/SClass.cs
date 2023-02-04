using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SaturnBuildTool
{
    // Vaild arguments for SCLASS macro
    [Flags]
    public enum SC
    {
        None = 0,
        Spawnable = 1 << 0,
        VisibleInEditor = 1 << 1
    }
}
