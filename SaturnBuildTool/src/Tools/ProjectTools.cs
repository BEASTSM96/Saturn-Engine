using System;
using System.IO;

namespace SaturnBuildTool.Tools
{
    internal class ProjectTools
    {
        public string GetProjectPath(string ProjectName) 
        {
            string WorkingDir = Directory.GetCurrentDirectory();
            WorkingDir = Directory.GetParent(WorkingDir).ToString();
            WorkingDir = Directory.GetParent(WorkingDir).ToString();

            WorkingDir += ProjectName;

            string ProjectPath = WorkingDir + ProjectName;
            ProjectPath = Path.ChangeExtension(ProjectPath, "vcxproj").Trim();

            return ProjectPath;
        }
    }
}
