using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

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

            vswhere.WaitForExit();

            return vswhere.StandardOutput.ReadToEnd().Trim();
        }

        public static string FindMSVCToolsDir()
        {
            string VSWherePath = FindVSRootDir();
            string CLLocation = Path.Combine( VSWherePath, "VC\\Tools\\MSVC\\" );

            // Go up by one dir, as we don't know the what the version is.

            foreach (string d in Directory.GetDirectories(CLLocation))
            {
                // There is only one folder (most likely), so we can just return the first one as it should be the best version.
                return d;
            }

            return "";
        }
    }
}
