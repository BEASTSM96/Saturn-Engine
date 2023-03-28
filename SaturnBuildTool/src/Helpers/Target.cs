using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SaturnBuildTool
{
    public enum TargetKind
    {
        Win64,
        Win86,
        Unknown
    }

    internal class Target
    {
        public static readonly Target Instance = new Target();

        public string TargetName { get; set; }

        public void Init(string targetName) 
        {
            TargetName = targetName;
            TargetName = TargetName.Replace("/", string.Empty);
        }

        public TargetKind GetTargetKind() 
        {
            if( TargetName == "Win64" )
                return TargetKind.Win64;
            else if ( TargetName == "Win86" )
                return TargetKind.Win86;

            return TargetKind.Unknown;
        }
    }
}
