using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;

// This file does need a rework.
// When we compile we should have the Worker thread generate the file information, but we make sure that we don't write the to file.
namespace SaturnBuildTool.Tools
{
    internal class HeaderTool
    {
        public struct Property 
        {
            public string Name;
            public string Type;

            public int Line;
            public SP Flags;
        }

        public struct CurrentGenFile 
        {
            public SC SClassInfo;
            public string ClassName;
            public string BaseClass;

            // List of all properties with the SPROPERTY macro.
            public Dictionary<int, Property> Properties;
        }

        public HeaderTool()
        {
        }

        public static readonly HeaderTool Instance = new HeaderTool();

        // If the last line had a SPROPERTY macro.
        public bool LastLineHadSP;

        public enum CommandType 
        {
            Header,
            Source
        }

        public struct Command
        {
            public StringBuilder GeneratedHeader;
            public StringBuilder GeneratedSource;

            public CurrentGenFile CurrentFile;
            public CommandType Type;
            public string GenFilepath;
        }

        // Filepath, Command
        private readonly Dictionary<string, Command> CommandQueue = new Dictionary<string, Command>();

        public bool IsFilepathGeneratedH(string Filepath) 
        {
            return Filepath.Contains( ".Gen.h" );
        }

        public bool IsFilepathGeneratedS(string Filepath)
        {
            return Filepath.Contains(".Gen.cpp");
        }

        private string GetFirstBaseClass(string FileBuffer) 
        {
            using (StreamReader sr = new StreamReader(FileBuffer)) 
            {
                string buffer = sr.ReadToEnd().Trim();

                var classRegex = new Regex(@"class\s+(\w+)\s*(:\s*public\s+(\w+))?");
                var matches = classRegex.Matches(buffer);

                foreach (Match match in matches)
                {
                    if (match.Groups[3].Success)
                    {
                        return match.Groups[3].Value;
                    }
                }

                sr.Close();
            }

            // regex: "(?<=class\s)[\w]+(?=\s:)"

            return "";
        }

        private string GetClassName(string Line)
        {
            Match match = Regex.Match(Line, "class (\\w+)");

            if (match.Success) 
            {
                return match.Groups[1].Value;
            }

            return "";
        }

        private bool CanFileBeReflected(string HeaderPath)
        {
            bool AnyReflectiveCodeFound = false;

            try
            {
                using (StreamReader sr = new StreamReader(HeaderPath))
                {
                    string line;
                    while ((line = sr.ReadLine()) != null)
                    {
                        if (!LineIsNotComment(line))
                            {
                            continue;
                        }

                        string refleciblePattern = @"(class|struct)\s+\w+";

                        if (Regex.IsMatch(line, refleciblePattern))
                        {
                            if (Regex.IsMatch(line, "class"))
                            {
                                AnyReflectiveCodeFound = true;
                                break;
                            }
                            else if (Regex.IsMatch(line, "struct"))
                            {
                                AnyReflectiveCodeFound = true;
                                break;
                            }
                        }
                    }

                    sr.Close();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }

            return AnyReflectiveCodeFound;
        }

        private bool NoReflectInFile(string File)
        {
            bool Found = false;

            try
            {
                using (StreamReader sr = new StreamReader(File))
                {
                    string line;
                    while ((line = sr.ReadLine()) != null && !Found)
                    {
                        if (!LineIsNotComment(line))
                        {
                            continue;
                        }

                        if (line.Contains("HT_FLAGS")) 
                        {
                            if (line.Contains("NoReflect")) 
                            {
                                Console.WriteLine(string.Format("'NoReflect' flag was found in {0}, skipping.", Path.GetFileName(File)));

                                Found = true;
                            }
                        }
                    }

                    sr.Close();
                }
            } 
            catch(Exception ex) 
            {
                Console.WriteLine(ex);
            }

            return Found;
        }

        private bool LineIsNotComment( string Line ) 
        {
            Match match = Regex.Match(Line, @"^\s*\/\/.*|^\s*\/\*.*\*\/\s*$");

            return !match.Success;
        }

        private bool GenerateHeader(string GenHeaderPath, string HeaderPath) 
        {
            bool Result = false;

            FileStream fs = new FileStream(GenHeaderPath, FileMode.Open, FileAccess.Write, FileShare.ReadWrite);
            fs.SetLength(0);
            fs.Close();

            string GenWarning = "/* Generated code, DO NOT modify! */";

            Command cmd;
            CommandQueue.TryGetValue(HeaderPath, out cmd);
            cmd.CurrentFile = new CurrentGenFile();

            // Reset.
            string FirstBaseClass = GetFirstBaseClass( HeaderPath );
            cmd.CurrentFile.BaseClass = FirstBaseClass;

            // Reset the class name.
            cmd.CurrentFile.ClassName = null;
            cmd.CurrentFile.Properties = new Dictionary<int, Property>();

            cmd.GeneratedHeader = new StringBuilder();

            try
            {
                StreamReader sr = new StreamReader(HeaderPath);

                string line;
                int lineNumber = 0;

                cmd.GeneratedHeader.AppendLine(GenWarning);

                cmd.GeneratedHeader.AppendLine("#pragma once\r\n");

                cmd.GeneratedHeader.AppendLine(string.Format("#include \"{0}\"", "Saturn/GameFramework/GameScript.h"));

                cmd.GeneratedHeader.AppendLine("\r\n");

                while ((line = sr.ReadLine()) != null)
                {
                    lineNumber++;

                    // SPROPERTY
                    if (LastLineHadSP)
                    {
                        // Now we (should), we at the line where the value is defined, we can now parse it.
                        Regex regex = new Regex(@"^\s*(?<type>\w+(?:\[\])?)\s+(?<name>\w+)\s*(?:=\s*\S+\s*)?;?$");
                        Match m = regex.Match(line);

                        if (m.Success)
                        {
                            string type = m.Groups["type"].Value;
                            string name = m.Groups["name"].Value;

                            Property p = new Property
                            {
                                Name = name,
                                Type = type,
                                Line = lineNumber
                            };

                            if (cmd.CurrentFile.Properties.ContainsKey(lineNumber))
                            {
                                cmd.CurrentFile.Properties.TryGetValue(lineNumber, out Property oldProp);
                                oldProp = p;
                            }
                            else
                            {
                                cmd.CurrentFile.Properties[lineNumber] = p;
                            }
                        }

                        LastLineHadSP = false;
                    }

                    if (line.Contains("SCLASS") && LineIsNotComment(line))
                    {
                        if (line.Contains("Spawnable"))
                        {
                            cmd.CurrentFile.SClassInfo |= SC.Spawnable;
                        }

                        if (line.Contains("VisibleInEditor"))
                        {
                            cmd.CurrentFile.SClassInfo |= SC.VisibleInEditor;
                        }
                    }

                    if (line.Contains("class") && cmd.CurrentFile.ClassName == null && LineIsNotComment(line))
                    {
                        cmd.CurrentFile.ClassName = GetClassName(line);
                    }

                    if (line.Contains("GENERATED_BODY") && LineIsNotComment(line))
                    {
                        // If we have found the GENERATED_BODY, we now need to define our CURRENT_FILE_ID, in out Gen file.
                        string baseFileId = "FID_";
                        baseFileId += Path.GetFileNameWithoutExtension(HeaderPath);
                        baseFileId += "_h_";
                        baseFileId += lineNumber.ToString();

                        string CFI = "#undef CURRENT_FILE_ID\r\n#define CURRENT_FILE_ID FID_";
                        CFI += Path.GetFileNameWithoutExtension(HeaderPath);
                        CFI += "_h"; // always end with _h

                        cmd.GeneratedHeader.AppendLine("\r\n");

                        cmd.GeneratedHeader.AppendLine(CFI);

                        // Now we define the real, GENERATED_BODY macro.
                        string idGeneratedBody = "#define ";
                        idGeneratedBody += baseFileId;
                        idGeneratedBody += "_GENERATED_BODY \\\r\n";

                        // Before we can write the macro, we need to create a new macro contiaing the class declarations.

                        string classDeclarations = "#define ";
                        classDeclarations += baseFileId;
                        classDeclarations += "_CLASSDECLS \\\r\n";

                        classDeclarations += "private: \\\r\n";
                        classDeclarations += "\tDECLARE_CLASS(";
                        classDeclarations += Path.GetFileNameWithoutExtension(HeaderPath);
                        classDeclarations += ", " + FirstBaseClass;
                        classDeclarations += ") \\\r\n";
                        classDeclarations += "public:";

                        cmd.GeneratedHeader.AppendLine("\r\n");
                        cmd.GeneratedHeader.AppendLine(classDeclarations);

                        idGeneratedBody += baseFileId;
                        idGeneratedBody += "_CLASSDECLS \r\n";

                        cmd.GeneratedHeader.AppendLine("\r\n");
                        cmd.GeneratedHeader.AppendLine(idGeneratedBody);
                    }

                    if (line.Contains("SPROPERTY") && LineIsNotComment(line))
                    {
                        LastLineHadSP = true;

                        Property p = new Property
                        {
                            Line = lineNumber + 1
                        };

                        if (!cmd.CurrentFile.Properties.ContainsKey(p.Line) || !cmd.CurrentFile.Properties.ContainsValue(p))
                        {
                            cmd.CurrentFile.Properties.Add(p.Line, p);
                        }

                        if (line.Contains("VisibleInEditor"))
                        {
                            p.Flags |= SP.VisibleInEditor;
                        }
                    }
                }
            }
            catch(Exception e) 
            {
               Console.WriteLine("Error when trying to generate header:");
               Console.WriteLine( e.Message );
            }

            WriteGeneratedHeader(GenHeaderPath, cmd.GeneratedHeader);

            CommandQueue[HeaderPath] = cmd;

            return Result;
        }

        private bool GenerateSource(string GenSourcePath, string HeaderPath) 
        {
            bool Result = false;

            FileStream fs = new FileStream(GenSourcePath, FileMode.Open, FileAccess.Write, FileShare.ReadWrite);
            fs.SetLength(0);
            fs.Close();

            string GenWarning = "/* Generated code, DO NOT modify! */";

            Command cmd;
            CommandQueue.TryGetValue(HeaderPath, out cmd);
            cmd.GeneratedSource = new StringBuilder();

            try
            {
                cmd.GeneratedSource.AppendLine(GenWarning);

                cmd.GeneratedSource.AppendLine(string.Format("#include \"{0}\"", string.Format( "{0}.Gen.h", cmd.CurrentFile.ClassName ) ));

                cmd.GeneratedSource.AppendLine(string.Format("#include \"{0}\"", "Saturn/GameFramework/GameScript.h"));
                cmd.GeneratedSource.AppendLine(string.Format("#include \"{0}\"", "Saturn/GameFramework/GamePrefabList.h"));
                cmd.GeneratedSource.AppendLine(string.Format("#include \"{0}\"", "Saturn/GameFramework/EntityScriptManager.h"));
                cmd.GeneratedSource.AppendLine(string.Format("#include \"{0}\"", "Saturn/Scene/Entity.h"));

                cmd.GeneratedSource.AppendLine(string.Format("#include \"{0}\"\r\n", string.Format( "{0}.h", cmd.CurrentFile.ClassName )));

                string cExternBeg = "extern \"C\" {\r\n";
                string cExternEnd = "}\r\n";

                cmd.GeneratedSource.AppendLine(cExternBeg);

                // Check if this class is spawnable
                if ((cmd.CurrentFile.SClassInfo & SC.Spawnable) == SC.Spawnable)
                {
                    // If our class in spawnable we can now create the spawn function.

                    string function = "__declspec(dllexport) Saturn::Entity* _Z_Create_";
                    function += cmd.CurrentFile.ClassName;
                    function += "()\r\n";
                    function += "{\r\n";
                    function += "\t return ";
                    function += cmd.CurrentFile.ClassName;
                    function += "::Spawn();\r\n";
                    function += "}\r\n";

                    cmd.GeneratedSource.AppendLine(function);

                    string functionBaseClass = "__declspec(dllexport) Saturn::Entity* _Z_Create_";
                    functionBaseClass += cmd.CurrentFile.ClassName;
                    functionBaseClass += "_FromBase";
                    functionBaseClass += string.Format("({0}* Ty)\r\n", cmd.CurrentFile.BaseClass);
                    functionBaseClass += "{\r\n";
                    functionBaseClass += "\t return new ";
                    functionBaseClass += cmd.CurrentFile.ClassName;
                    functionBaseClass += "(*Ty);\r\n";
                    functionBaseClass += "}\r\n";

                    cmd.GeneratedSource.AppendLine(functionBaseClass);

                    cmd.GeneratedSource.AppendLine("//^^^ Spawnable\r\n");
                }
                else 
                {
                    string function = "__declspec(dllexport) Saturn::Entity* _Z_Create_";
                    function += cmd.CurrentFile.ClassName;
                    function += "()\r\n";
                    function += "{\r\n";
                    function += "\treturn nullptr;\r\n";
                    function += "}\r\n";

                    cmd.GeneratedSource.AppendLine(function);

                    string functionBaseClass = "__declspec(dllexport) Saturn::Entity* _Z_Create_";
                    functionBaseClass += cmd.CurrentFile.ClassName;
                    functionBaseClass += "_FromBase";
                    functionBaseClass += string.Format("({0}* Ty)\r\n", cmd.CurrentFile.BaseClass);
                    functionBaseClass += "{\r\n";
                    functionBaseClass += "\treturn nullptr;\r\n";
                    functionBaseClass += "}\r\n";

                    cmd.GeneratedSource.AppendLine(functionBaseClass);

                    cmd.GeneratedSource.AppendLine("//^^^ NO Spawnable\r\n");
                }

                cmd.GeneratedSource.AppendLine(cExternEnd);

                // Add to prefab list
                string prefab = string.Format("static void _RT_Z_AddPrefab_{0}()\r\n", cmd.CurrentFile.ClassName);
                prefab += "{\r\n";
                prefab += string.Format("\t Saturn::GamePrefabList::Get().Add(\"{0}\");\r\n", cmd.CurrentFile.ClassName);
                prefab += "}\r\n";
                prefab += "//^^^ Type is prefab...\r\n";

                cmd.GeneratedSource.AppendLine(prefab);

                // Properties
                string propfunc = string.Format("static void _Zp_{0}_Reg_Props()\r\n", cmd.CurrentFile.ClassName);
                propfunc += "{\r\n";

                foreach (KeyValuePair<int, Property> kv in cmd.CurrentFile.Properties)
                {
                    Property property = kv.Value;

                    if (property.Name == null)
                        {
                        continue;
                    }

                    propfunc += string.Format("\t// [_Zp_Public_Prop] {0} of type {1}\r\n", property.Name.ToUpper(), property.Type.ToUpper());
                    propfunc += string.Format("\t//Saturn::EntityScrviptManager::Get().AddProperty( \"{0}\", \"{1}\", (void*)&{2}::{1});\r\n", cmd.CurrentFile.ClassName, property.Name, cmd.CurrentFile.ClassName);
                }

                propfunc += "}\r\n";
                propfunc += "//^^^ Public Properties\r\n";

                cmd.GeneratedSource.AppendLine(propfunc);

                // Auto-Registration (DLL only).
                if (BuildConfig.Instance.GetTargetConfig() < ConfigKind.DistDebug)
                {
                    Random random = new Random();
                    int randomNumber = random.Next();
                        
                    string call = string.Format("struct _Z_{0}_RT_Editor\r\n", cmd.CurrentFile.ClassName);
                    call += "{\r\n";
                    call += string.Format("\t_Z_{0}_RT_Editor()\r\n", cmd.CurrentFile.ClassName);
                    call += "\t{\r\n";
                    //call += string.Format("\t\t_RT_Z_Add{0}ToEditor();\r\n", CurrentFile.ClassName);
                    call += string.Format("\t\t_RT_Z_AddPrefab_{0}();\r\n", cmd.CurrentFile.ClassName);
                    call += string.Format("\t\t_Zp_{0}_Reg_Props();\r\n", cmd.CurrentFile.ClassName);
                    call += "\r\n";
                    call += "\t}\r\n";
                    call += "};\r\n";
                    call += "\r\n";
                    call += string.Format("static _Z_{0}_RT_Editor _Z_RT_{0}_{1};", cmd.CurrentFile.ClassName, randomNumber);

                    cmd.GeneratedSource.AppendLine(call);
                    cmd.GeneratedSource.AppendLine("//^^^ Auto-Registration\r\n");
                }
                else
                {
                    string call = "// No Auto-Registration Game is exe.\r\n";
                    cmd.GeneratedSource.AppendLine(call);
                }
            }
            catch (Exception e) 
            {
                Console.WriteLine("Error when generating source:");
                Console.WriteLine(e.Message);
            }
  
            WriteGeneratedSource(GenSourcePath, cmd.GeneratedSource);

            CommandQueue[HeaderPath] = cmd;

            return Result;
        }

        public bool GenerateSource(string SourcePath)
        {
            // Assume the Filepath is a source file
            string GenFilepath = Path.ChangeExtension(SourcePath, ".Gen.cpp");
            string HeaderFilepath = Path.ChangeExtension(SourcePath, ".h");

            if (!File.Exists(HeaderFilepath)) 
            {
                return false;
            }

            if (IsFilepathGeneratedS(HeaderFilepath))
            {
                return false;
            }

            if (!CanFileBeReflected(HeaderFilepath))
            {
                return false;
            }

            if (NoReflectInFile(HeaderFilepath))
            {
                return false;
            }

            if (!CommandQueue.ContainsKey(HeaderFilepath))
            {
                Command cmd = new Command
                {
                    Type = CommandType.Source,
                    GenFilepath = GenFilepath
                };

                try
                {
                    CommandQueue.Add(HeaderFilepath, cmd);
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.Message);
                }
            }
            else
            {
                Command cmd;
                CommandQueue.TryGetValue(HeaderFilepath, out cmd);

                cmd.Type |= CommandType.Source;

                CommandQueue[HeaderFilepath] = cmd;
            }

            if (!File.Exists(GenFilepath))
            {
                File.Create(GenFilepath).Close();
            }

            GenerateSource(GenFilepath, HeaderFilepath);

            return false;
        }

        public bool GenerateHeader(string Filepath)
        {
            // Assume the Filepath is a source file
            string HeaderFilepath = Path.ChangeExtension(Filepath, ".h");
            string GenFilepath = Path.ChangeExtension(HeaderFilepath, ".Gen.h");

            if (!File.Exists(HeaderFilepath))
            {
                return false;
            }

            if (IsFilepathGeneratedH(HeaderFilepath))
            {
                return false;
            }

            if (!CanFileBeReflected(HeaderFilepath))
            {
                return false;
            }

            if (NoReflectInFile(HeaderFilepath))
            {
                return false;
            }

            if (!CommandQueue.ContainsKey(HeaderFilepath))
            {
                Command cmd = new Command
                {
                    Type = CommandType.Header,
                    GenFilepath = GenFilepath
                };

                try
                {
                    CommandQueue.Add(HeaderFilepath, cmd);
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.Message);
                }
            }
            else
            {
            }

            if (!File.Exists(GenFilepath)) 
            {
                File.Create(GenFilepath).Close();
            }

            GenerateHeader( GenFilepath, HeaderFilepath);

            return false;
        }

        public void ExecuteAll() 
        {
            foreach (KeyValuePair<string, Command> kv in CommandQueue) 
            {
                string Filepath = kv.Key;
                Command cmd = kv.Value;

                switch (cmd.Type) 
                {
                    case CommandType.Source:
                        {
                            string GenFilepath = Path.ChangeExtension(cmd.GenFilepath, ".Gen.cpp");

                            // Safe to create the file.
                            FileStream fs = null;
                            if (!File.Exists(GenFilepath))
                            {
                                fs = File.Create(GenFilepath);
                            }

                            if (fs != null)
                            {
                                fs.Close();
                            }

                            GenerateSource(Filepath, GenFilepath);
                        } break;

                    case CommandType.Header:
                        {
                            // Safe to create the file.
                            FileStream fs = null;
                            if (!File.Exists(cmd.GenFilepath))
                            {
                                fs = File.Create(cmd.GenFilepath);
                            }

                            if (fs != null)
                            {
                                fs.Close();
                            }

                            GenerateHeader(cmd.GenFilepath, Filepath);
                        } break;
                }
            }
        }

        private void WriteGeneratedHeader(string Filepath, StringBuilder Contents) 
        {
            //string GenFilepath = Path.ChangeExtension( Filepath, ".Gen.h" );

            FileStream fs = new FileStream(Filepath, FileMode.Open, FileAccess.Write, FileShare.ReadWrite );
            fs.SetLength(0);

            StreamWriter streamWriter = new StreamWriter(fs);

            streamWriter.WriteLine(Contents.ToString());

            streamWriter.Close();
            fs.Close();
        }

        private void WriteGeneratedSource(string Filepath, StringBuilder Contents)
        {
            //string GenFilepath = Path.ChangeExtension(Filepath, ".Gen.cpp");

            FileStream fs = new FileStream(Filepath, FileMode.Open, FileAccess.Write, FileShare.ReadWrite);
            fs.SetLength(0);

            StreamWriter streamWriter = new StreamWriter(fs);

            streamWriter.WriteLine(Contents.ToString());

            streamWriter.Close();
            fs.Close();
        }

        private void WriteGeneratedFile(string Path, Command command) 
        {
            switch (command.Type) 
            {
                case CommandType.Header:
                    {
                        WriteGeneratedHeader(Path, command.GeneratedHeader);
                    } break;

                case CommandType.Source:
                    {
                        WriteGeneratedSource(Path, command.GeneratedSource);
                    }
                    break;
            }
        }

        public void WriteGeneratedFiles()
        {
            foreach (KeyValuePair<string, Command> kv in CommandQueue)
            {
                string filepath = kv.Key;
                Command cmd = kv.Value;

                WriteGeneratedFile(filepath, cmd);
            }
        }

        public List<string> GetAllGeneratedFiles()
        {
            List<string> generatedFiles = new List<string>();

            foreach (KeyValuePair<string, Command> kv in CommandQueue)
            {
                Command cmd = kv.Value;

                generatedFiles.Add(cmd.GenFilepath);
            }
            
            return generatedFiles;
        }
    }
}
