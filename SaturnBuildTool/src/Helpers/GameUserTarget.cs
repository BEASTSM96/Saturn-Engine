using BuildTool;
using System;
using System.IO;

namespace BuildTool
{
    public class GameUserTarget : UserTarget
    {
        public override void Init() 
        {
            base.Init();

            // Always use x64
            Architectures = new[] { TargetKind.Win64 };

            BuildConfigs = new[] { ConfigKind.Debug, ConfigKind.Release, ConfigKind.Dist };

            // Games are DLLs in the editor
            // TODO: Allow for games to be an exe.
            OutputType = LinkerOutput.SharedLibrary;

            // Include source.
            Includes.Add( "src" );

            // Saturn:
            string saturnRootDir = Environment.GetEnvironmentVariable( "SATURN_DIR" );

            string saturnSingletonDir = saturnRootDir;
            saturnSingletonDir = Path.Combine(saturnSingletonDir, "SingletonStorage" );

            string saturnVendor = Path.Combine( saturnRootDir, "Saturn\\vendor" );
            string saturnSrc = Path.Combine( saturnRootDir, "Saturn\\src" );

            Includes.Add( saturnSrc );

            // Saturn Vendor
            Includes.Add( Path.Combine(saturnVendor, "spdlog\\include") );
            Includes.Add( Path.Combine(saturnVendor, "vulkan\\include") );
            Includes.Add( Path.Combine(saturnVendor, "glm") );
            Includes.Add( Path.Combine(saturnVendor, "GLFW\\include") );
            Includes.Add( Path.Combine(saturnVendor, "imgui") );
            Includes.Add( Path.Combine(saturnVendor, "entt\\include") );
            Includes.Add( Path.Combine(saturnVendor, "assimp\\include") );
            Includes.Add( Path.Combine(saturnVendor, "shaderc\\libshaderc\\include") );
            Includes.Add( Path.Combine(saturnVendor, "SPRIV-Cross\\src") );
            Includes.Add( Path.Combine(saturnVendor, "vma\\src") );
            Includes.Add( Path.Combine(saturnVendor, "ImGuizmo\\src") );
            Includes.Add( Path.Combine(saturnVendor, "yaml-cpp\\include") );
            Includes.Add( Path.Combine(saturnVendor, "imgui_node_editor") );
            Includes.Add( Path.Combine(saturnVendor, "physx\\include") );
            Includes.Add( Path.Combine(saturnVendor, "physx\\include\\pxshared") );
            Includes.Add( Path.Combine(saturnVendor, "physx\\include\\physx") );
            Includes.Add( Path.Combine(saturnSingletonDir, "src") );

            string saturnBinDir = saturnRootDir;
            saturnBinDir = Path.Combine(saturnBinDir, "bin" );

            switch ( CurrentConfig ) 
            {
                case ConfigKind.Debug: 
                    {
                        saturnBinDir = Path.Combine(saturnBinDir, "Debug-windows-x86_64\\Saturn" );
                    } break;

                case ConfigKind.Release:
                    {
                        saturnBinDir = Path.Combine(saturnBinDir, "Release-windows-x86_64\\Saturn");
                    }
                    break;

                case ConfigKind.Dist:
                    {
                        saturnBinDir = Path.Combine(saturnBinDir, "Dist-windows-x86_64\\Saturn");
                    }
                    break;
            }

            string libPath = saturnBinDir;

            // Link to saturn
            Links.Add("Saturn.lib");

            LibraryPaths.Add( libPath );
        }
    }
}
