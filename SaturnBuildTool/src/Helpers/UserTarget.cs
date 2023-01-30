using System;
using System.Collections.Generic;
using System.Reflection;
using System.CodeDom.Compiler;
using System.Linq;
using System.IO;

using BuildTool.Tools;

namespace BuildTool
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

        public LinkerOutput OutputType = LinkerOutput.Executable;

        //bool IsGame = false;

        public virtual void Init() 
        {
        }

        public List<string> GetIntermediateFiles() 
        {
            return DirectoryTools.DirSearch(OutputPath, ".obj");
        }

        public string GetBinDir() 
        {
            string BinDir = Directory.GetParent( OutputPath ).ToString();
            BinDir = Path.Combine( BinDir, "bin" );

            switch (CurrentConfig)
            {
                case ConfigKind.Debug:
                    {
                        BinDir = Path.Combine(BinDir, "Debug");
                    }
                    break;

                case ConfigKind.Release:
                    {
                        BinDir = Path.Combine(BinDir, "Release");
                    }
                    break;

                case ConfigKind.Dist:
                    {
                        BinDir = Path.Combine(BinDir, "Dist");
                    }
                    break;
            }

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
            
            Assembly[] DefaultReferences = { typeof(Enumerable).Assembly, typeof(ISet<>).Assembly, typeof(UserTarget).Assembly };

            HashSet<string> references = new HashSet<string>();
            foreach (var defaultReference in DefaultReferences)
                references.Add(defaultReference.Location);

            CompilerParameters cp = new CompilerParameters();
            cp.GenerateExecutable = false;
            cp.WarningLevel = 3;
            cp.TreatWarningsAsErrors = false;
            cp.ReferencedAssemblies.AddRange( references.ToArray() );

            CompilerResults cr = codeDomProvider.CompileAssemblyFromFile(cp, BuildFile);

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
                                outDir = Path.Combine( outDir, "obj" );
                                outDir = Path.Combine( outDir, "Debug" );
                            }
                            break;

                        case ConfigKind.Release:
                            {
                                outDir = Path.Combine(outDir, "obj");
                                outDir = Path.Combine(outDir, "Release");
                            }
                            break;

                        case ConfigKind.Dist:
                            {
                                outDir = Path.Combine(outDir, "obj");
                                outDir = Path.Combine(outDir, "Dist");
                            }
                            break;
                    }

                    target.OutputPath = outDir;

                    // Init the target.
                    target.Init();
                }
            }

            return target;
        }
    }
}
