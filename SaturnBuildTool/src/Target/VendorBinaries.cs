using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SaturnBuildTool
{
    enum VendorProject 
    {
        GLFW,
        PHSYX,
        YAML_CPP,
        IMGUI,
        SPIRVCROSS,
        SHADERC,
        OPTICK
    }

    internal class VendorBinaries
    {
        private static string GetRootBinPath(string path, UserTarget target) 
        {
            switch (target.CurrentConfig) 
            {
                case ConfigKind.Debug: 
                    {
                        path = Path.Combine(path, "Debug-windows-x86_64");
                    } break;

                case ConfigKind.Release:
                    {
                        path = Path.Combine(path, "Release-windows-x86_64");
                    }
                    break;

                case ConfigKind.Dist:
                    {
                        path = Path.Combine(path, "Dist-windows-x86_64");
                    }
                    break;
                
                default:
                    break;
            }

            return path;
        }

        // TODO
        // TEMP
        public static string GetBinPath(VendorProject project, UserTarget target) 
        {
            string saturnDir = Environment.GetEnvironmentVariable("SATURN_DIR");
            string binPath = Path.Combine(saturnDir, "Saturn\\vendor");

            switch ( project ) 
            {
                case VendorProject.GLFW:
                    {
                        binPath = Path.Combine(binPath, "GLFW\\bin\\" );

                        binPath = GetRootBinPath(binPath, target);

                        binPath = Path.Combine(binPath, "GLFW" );

                    } break;


                case VendorProject.PHSYX:
                    {
                        binPath = Path.Combine(binPath, "phsyx\\bin\\");

                        if(target.CurrentConfig == ConfigKind.Debug)
                            binPath = Path.Combine(binPath, "Debug");
                        else
                            binPath = Path.Combine(binPath, "Release");

                    }
                    break;


                case VendorProject.YAML_CPP:
                    {
                        binPath = Path.Combine(binPath, "yaml-cpp\\bin\\");

                        binPath = GetRootBinPath(binPath, target);

                        binPath = Path.Combine(binPath, "yaml-cpp");
                    }
                    break;


                case VendorProject.IMGUI:
                    {
                        binPath = Path.Combine(binPath, "imgui\\bin\\");

                        binPath = GetRootBinPath(binPath, target);

                        binPath = Path.Combine(binPath, "ImGui");
                    }
                    break;


                case VendorProject.SPIRVCROSS:
                    {
                        binPath = Path.Combine(binPath, "SPIRV-Cross\\bin\\");

                        binPath = GetRootBinPath(binPath, target);

                        binPath = Path.Combine(binPath, "SPIRV-Cross");
                    }
                    break;


                case VendorProject.SHADERC:
                    {
                        binPath = Path.Combine(binPath, "shaderc\\bin\\");

                        if (target.CurrentConfig == ConfigKind.Debug)
                            binPath = Path.Combine(binPath, "Debug-Windows");
                        else
                            binPath = Path.Combine(binPath, "Release-Windows");
                    }
                    break;

                case VendorProject.OPTICK:
                    {
                        binPath = Path.Combine(binPath, "optick\\bin\\");

                        binPath = GetRootBinPath(binPath, target);

                        binPath = Path.Combine(binPath, "optick");
                    }
                    break;
            }

            return binPath;
        }
    }
}
