﻿using SaturnBuildTool;
using System;
using System.IO;

namespace SaturnBuildTool
{
    public class GameUserTarget : UserTarget
    {
        protected string SaturnRootDir = "";
        protected string SaturnSingletonDir = "";
        protected string SaturnSourceDir = "";
        protected string SaturnVendorDir = "";

        public override void Init() 
        {
            base.Init();

            // Always use x64
            Architectures = new[] { ArchitectureKind.x64 };

            // Include source.
            Includes.Add( ProjectInfo.Instance.SourceDir );
            Includes.Add( Path.Combine( ProjectInfo.Instance.SourceDir, ProjectInfo.Instance.Name ) );
            // Include Build folder for generated files.
            Includes.Add( ProjectInfo.Instance.BuildDir );

            // Saturn:
            SaturnRootDir = Environment.GetEnvironmentVariable( "SATURN_DIR" );

            SaturnSingletonDir = Path.Combine(SaturnRootDir, "SharedStorage");

            SaturnVendorDir = Path.Combine(SaturnRootDir, "Saturn\\vendor" );
            SaturnSourceDir = Path.Combine(SaturnRootDir, "Saturn\\src" );

            Includes.Add(SaturnSourceDir);

            // Saturn Vendor
            Includes.Add( Path.Combine(SaturnVendorDir, "spdlog\\include") );
            Includes.Add( Path.Combine(SaturnVendorDir, "vulkan\\include") );
            Includes.Add( Path.Combine(SaturnVendorDir, "glm") );
            Includes.Add( Path.Combine(SaturnVendorDir, "ruby\\src") );
            Includes.Add( Path.Combine(SaturnVendorDir, "imgui") );
            Includes.Add( Path.Combine(SaturnVendorDir, "entt\\include") );
            Includes.Add( Path.Combine(SaturnVendorDir, "assimp\\include") );
            Includes.Add( Path.Combine(SaturnVendorDir, "shaderc\\libshaderc\\include") );
            Includes.Add( Path.Combine(SaturnVendorDir, "SPRIV-Cross\\src") );
            Includes.Add( Path.Combine(SaturnVendorDir, "vma\\src") );
            Includes.Add( Path.Combine(SaturnVendorDir, "ImGuizmo\\src") );
            Includes.Add( Path.Combine(SaturnVendorDir, "yaml-cpp\\include") );
            Includes.Add( Path.Combine(SaturnVendorDir, "imgui_node_editor") );
            Includes.Add( Path.Combine(SaturnVendorDir, "physx\\include") );
            Includes.Add( Path.Combine(SaturnVendorDir, "physx\\include\\pxshared") );
            Includes.Add( Path.Combine(SaturnVendorDir, "physx\\include\\physx") );
            Includes.Add( Path.Combine(SaturnVendorDir, "tracy\\src") );
            Includes.Add( Path.Combine(SaturnVendorDir, "miniaudio\\src") );
            Includes.Add( Path.Combine(SaturnSingletonDir, "src") );

            string saturnBinDir = SaturnRootDir;
            string ssBinDir = SaturnRootDir;
            saturnBinDir = Path.Combine(saturnBinDir, "bin" );
            ssBinDir = Path.Combine(ssBinDir, "bin" );

            switch ( CurrentConfig ) 
            {
                case ConfigKind.Debug: 
                    {
                        saturnBinDir = Path.Combine(saturnBinDir, "Debug-windows-x86_64\\Saturn" );
                        ssBinDir = Path.Combine(ssBinDir, "Debug-windows-x86_64\\SharedStorage");
                    } break;

                case ConfigKind.Release:
                    {
                        saturnBinDir = Path.Combine(saturnBinDir, "Release-windows-x86_64\\Saturn");
                        ssBinDir = Path.Combine(ssBinDir, "Release-windows-x86_64\\SharedStorage");
                    }
                    break;

                case ConfigKind.Dist:
                    {
                        saturnBinDir = Path.Combine(saturnBinDir, "Dist-windows-x86_64\\Saturn");
                        ssBinDir = Path.Combine(ssBinDir, "Dist-windows-x86_64\\SharedStorage");
                    }
                    break;
            }

            string libPath = saturnBinDir;

            // Link to saturn
            Links.Add("Saturn.lib");
            Links.Add("SharedStorage.lib");

            PreprocessorDefines.Add("SATURN_SS_IMPORT");

            LibraryPaths.Add( libPath );
            LibraryPaths.Add( ssBinDir );

            for ( int i = 0; i < Enum.GetNames(typeof(VendorProject)).Length; i++ ) 
            {
                string path = VendorBinaries.GetBinPath( (VendorProject)i, this );

                LibraryPaths.Add(path);
            }

            // Vendor
            Links.Add("Ruby.lib");
            Links.Add("ImGui.lib");
            Links.Add("shaderc.lib");
            Links.Add("SPIRV-Cross.lib");
            Links.Add("yaml-cpp.lib");
            Links.Add("Tracy.lib");
            Links.Add("zlib.lib");
        }
    }
}
