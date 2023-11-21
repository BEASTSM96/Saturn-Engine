﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SaturnBuildTool
{
    enum VendorProject 
    {
        RUBY,
        PHYSX,
        YAML_CPP,
        IMGUI,
        SPIRVCROSS,
        SHADERC,
        TRACY
    }

    internal class VendorBinaries
    {
        private static string GetRootBinPath(string path, UserTarget target) 
        {
            switch (target.CurrentConfig) 
            {
                case ConfigKind.DistDebug: 
                case ConfigKind.Debug: 
                    {
                        path = Path.Combine(path, "Debug-windows-x86_64");
                    } break;

                case ConfigKind.DistRelease: 
                case ConfigKind.Release:
                    {
                        path = Path.Combine(path, "Release-windows-x86_64");
                    }
                    break;

                case ConfigKind.DistFull: 
                case ConfigKind.Dist:
                    {
                        path = Path.Combine(path, "Dist-windows-x86_64");
                    }
                    break;
            }

            return path;
        }

        // TODO
        // TEMP: There is 1000% a better way of doing this.
        public static string GetBinPath(VendorProject project, UserTarget target) 
        {
            string saturnDir = Environment.GetEnvironmentVariable("SATURN_DIR");
            string binPath = Path.Combine(saturnDir, "Saturn\\vendor");

            switch ( project ) 
            {
                case VendorProject.RUBY:
                    {
                        binPath = Path.Combine(binPath, "Ruby\\bin\\" );

                        binPath = GetRootBinPath(binPath, target);

                        binPath = Path.Combine(binPath, "Ruby");

                    } break;


                case VendorProject.PHYSX:
                    {
                        binPath = Path.Combine(binPath, "phsyx\\bin\\");

                        if (target.CurrentConfig == ConfigKind.Debug)
                        {
                            binPath = Path.Combine(binPath, "Debug");
                        }
                        else 
                        {
                            binPath = Path.Combine(binPath, "Release");
                        }
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
                        {
                            binPath = Path.Combine(binPath, "Debug-Windows");
                        }
                        else 
                        {
                            binPath = Path.Combine(binPath, "Release-Windows");
                        }
                    }
                    break;

                case VendorProject.TRACY:
                    {
                        binPath = Path.Combine(binPath, "tracy\\bin\\");

                        binPath = GetRootBinPath(binPath, target);

                        binPath = Path.Combine(binPath, "Tracy");
                    }
                    break;
            }

            return binPath;
        }
    }
}
