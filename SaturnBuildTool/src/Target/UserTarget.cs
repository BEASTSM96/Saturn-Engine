
using System;
using System.Collections.Generic;
using System.Reflection;
using System.CodeDom.Compiler;
using System.Linq;
using System.IO;

using SaturnBuildTool.Tools;

namespace SaturnBuildTool
{
    public class UserTarget
    {
        public TargetKind[] Architectures { get; set; }
        public ConfigKind[] BuildConfigs { get; set; }

        public ConfigKind CurrentConfig { get; set; }

        public string ProjectName { get; set; }

        public string OutputName { get; set; }

        public string OutputPath { get; set; }

        public string FilePath { get; set; }

        public List<string> Includes = new List<string>();

        public List<string> PreprocessorDefines = new List<string>();

        public List<string> Links = new List<string>();

        public List<string> DynamicBase = new List<string>();

        public List<string> LibraryPaths = new List<string>();

        public LinkerOutput OutputType = LinkerOutput.Executable;

        public virtual void Init() 
        {
            Links.Add("dwmapi.lib");
            Links.Add("kernel32.lib");
            Links.Add("user32.lib");
            Links.Add("comdlg32.lib");
            Links.Add("advapi32.lib");
            Links.Add("shell32.lib");
            Links.Add("ole32.lib");
            Links.Add("oleaut32.lib");
            Links.Add("delayimp.lib");
        }

        public List<string> GetIntermediateFiles() 
        {
            return DirectoryTools.DirSearch(OutputPath, ".obj");
        }

        public string GetBinDir() 
        {
            string BinDir = Directory.GetParent( OutputPath ).ToString();
            BinDir = Directory.GetParent( BinDir ).ToString();
            BinDir = Directory.GetParent( BinDir ).ToString();

            BinDir = Path.Combine( BinDir, "bin" );

            switch (CurrentConfig)
            {
                case ConfigKind.Debug:
                    {
                        BinDir = Path.Combine(BinDir, "Debug-windows-x86_64");
                    }
                    break;

                case ConfigKind.Release:
                    {
                        BinDir = Path.Combine(BinDir, "Release-windows-x86_64");
                    }
                    break;

                case ConfigKind.Dist:
                    {
                        BinDir = Path.Combine(BinDir, "Dist-windows-x86_64");
                    }
                    break;
            }

            BinDir = Path.Combine( BinDir, ProjectName );

            switch (OutputType)
            {
                case LinkerOutput.StaticLibrary:
                    {
                        BinDir = Path.Combine(BinDir, ProjectName);
                        BinDir = Path.ChangeExtension(BinDir, ".lib");
                    }
                    break;

                case LinkerOutput.SharedLibrary:
                    {
                        BinDir = Path.Combine(BinDir, ProjectName);
                        BinDir = Path.ChangeExtension(BinDir, ".dll");
                    }
                    break;

                case LinkerOutput.Executable:
                    {
                        BinDir = Path.Combine(BinDir, ProjectName);
                        BinDir = Path.ChangeExtension(BinDir, ".exe");
                    }
                    break;

                default:
                    break;
            }

            return BinDir;
        }

        public static UserTarget SetupUserTarget( string BuildFile ) 
        {
            UserTarget target = null;

            // Compile the *.Build.cs file
            Assembly asm;

            // Set up the CS build system.
            Dictionary<string, string> providerOptions = new Dictionary<string, string>();
            providerOptions.Add("CompilerVersion", "v4.0");
            CodeDomProvider codeDomProvider = new Microsoft.CSharp.CSharpCodeProvider(providerOptions);
            
            Assembly[] DefaultReferences = { typeof(IntPtr).Assembly, typeof(Enumerable).Assembly, typeof(ISet<>).Assembly, typeof(UserTarget).Assembly };

            HashSet<string> references = new HashSet<string>();
            foreach (var defaultReference in DefaultReferences)
                references.Add(defaultReference.Location);

            CompilerParameters cp = new CompilerParameters();
            cp.GenerateExecutable = false;
            cp.WarningLevel = 3;
            cp.TreatWarningsAsErrors = false;
            cp.GenerateInMemory = true;
            cp.IncludeDebugInformation = false;
            cp.ReferencedAssemblies.AddRange( references.ToArray() );

            CompilerResults cr = codeDomProvider.CompileAssemblyFromFile(cp, BuildFile);

            foreach (CompilerError ce in cr.Errors)
            {
                Console.WriteLine( ce.ErrorText );
            }
            
            asm = cr.CompiledAssembly;

            Type[] types = asm.GetTypes();

            for (var i = 0; i < types.Length; i++) 
            {
                var type = types[i];

                if (!type.IsClass || type.IsAbstract)
                    continue;

                if (type.IsSubclassOf(typeof(UserTarget))) 
                { 
                    // Create a user target.
                    target = (UserTarget)Activator.CreateInstance(type);

                    target.CurrentConfig = BuildConfig.Instance.GetTargetConfig();

                    string outDir = "";
                    outDir = Directory.GetParent(BuildFile).ToString();
                    outDir = Directory.GetParent(outDir).ToString();

                    switch (target.CurrentConfig)
                    {
                        case ConfigKind.Debug:
                            {
                                outDir = Path.Combine( outDir, "bin-int" );
                                outDir = Path.Combine( outDir, "Debug-windows-x86_64" );
                            }
                            break;

                        case ConfigKind.Release:
                            {
                                outDir = Path.Combine(outDir, "bin-int");
                                outDir = Path.Combine(outDir, "Release-windows-x86_64");
                            }
                            break;

                        case ConfigKind.Dist:
                            {
                                outDir = Path.Combine(outDir, "bin-int");
                                outDir = Path.Combine(outDir, "Dist-windows-x86_64");
                            }
                            break;

                        default:
                            break;
                    }

                    outDir = Path.Combine(outDir, Path.GetFileNameWithoutExtension( BuildFile ) );

                    // Remove .Build
                    int index = outDir.LastIndexOf('.');

                    outDir = outDir.Substring( 0, index );

                    target.OutputPath = outDir;

                    // Init the target.
                    target.Init();
                }
            }

            return target;
        }
    }
}
