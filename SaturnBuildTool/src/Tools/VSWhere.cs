using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace SaturnBuildTool.Tools
{
    internal class VSWhere
    {
        public static string FindVSRootDir() 
        {
            string VSWherePath = "C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe";

            ProcessStartInfo vswinfo = new ProcessStartInfo();
            vswinfo.FileName = VSWherePath;
            vswinfo.CreateNoWindow = true;
            vswinfo.Arguments = "-legacy -prerelease -latest -property installationPath";
            vswinfo.RedirectStandardOutput = true;
            vswinfo.RedirectStandardError = true;
            vswinfo.UseShellExecute = false;

            Process vswhere = new Process();
            vswhere.StartInfo = vswinfo;
            vswhere.Start();

            return vswhere.StandardOutput.ReadToEnd().Trim();
        }
    }
}
