using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using static SaturnBuildTool.Tools.HeaderTool;

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

        private bool ClassIsNotFwd( string Line ) 
        {
            // This regex checks for...
            // 1. That there is a namespace (i.e. Saturn::PhysicsRigidBody)
            // 2. Checks if it's actually a forward declaration.
            Match match = Regex.Match(Line, @"^\s*(?:namespace\s+\w+\s*{)?\s*class\s+\w+(?:::\w+)?\s*;\s*(?:})?\s*$");

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

                cmd.GeneratedHeader.AppendLine(string.Format("#include \"{0}\"", "Saturn/GameFramework/Core/GameScript.h"));

                cmd.GeneratedHeader.AppendLine("\r\n");

                while ((line = sr.ReadLine()) != null)
                {
                    lineNumber++;

                    if( LastLineHadSP ) 
                    {
                        // Parse args now.
                        Match typeMatch = Regex.Match(line, @"(\w+)\s+(\w+)");
                        if (typeMatch.Success)
                        {
                            string type = typeMatch.Groups[1].Value;
                            string name = typeMatch.Groups[2].Value;

                            Property p = new Property
                            {
                                Name = name,
                                Type = type,
                                Line = lineNumber
                            };

                            Console.WriteLine($"setting sprop details, line number: {lineNumber}");

                            cmd.CurrentFile.Properties.Add(lineNumber, p);
                        }

                        LastLineHadSP = false;
                    }

                    // Check for SPROPERTY
                    {
                        Match SpropertyMatch = Regex.Match( line, @"SPROPERTY\((.*?)\)", RegexOptions.Singleline );
                        if( SpropertyMatch.Success ) 
                        {
                            Console.WriteLine("found sprop");

                            // Now parse the args in the sprop
                            string args = SpropertyMatch.Groups[1].Value.Trim();
                            Console.WriteLine( $"Args: {args}" );

                            // Check if the line ends as we accept two type of valid use
                            // that is:
                            // SPROPERTY( args )
                            // float Test;
                            // OR:
                            // SPROPERTY( args ) float Test;

                            string renamingContentOnLine = line.Substring( SpropertyMatch.Index + SpropertyMatch.Length ).Trim();

                            if( renamingContentOnLine == string.Empty || renamingContentOnLine.StartsWith("\n") ) 
                            {
                                Console.WriteLine( "Line ends." );
                                LastLineHadSP = true;
                            }
                            else
                            {
                                // Parse args now.
                                Match typeMatch = Regex.Match(line, @"SPROPERTY\(.*?\)\s+(\w+)\s+(\w+)", RegexOptions.Singleline);
                                if (typeMatch.Success)
                                {
                                    string type = typeMatch.Groups[1].Value;
                                    string name = typeMatch.Groups[2].Value;

                                    Property p = new Property
                                    {
                                        Name = name,
                                        Type = type,
                                        Line = lineNumber
                                    };

                                    cmd.CurrentFile.Properties.Add( lineNumber, p );
                                }
                            }
                        }

                        /*
                        Console.WriteLine("checking sprop");

                        // Now we (should), we at the line where the value is defined, we can now parse it.
                        Regex regex = new Regex(@"SPROPERTY\(\)\s+(\w+)\s+(\w+)");
                        Match m = regex.Match(line);

                        if (m.Success)
                        {
                            string type = m.Groups[1].Value;
                            string name = m.Groups[2].Value;

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
                        
                            Console.WriteLine(cmd.CurrentFile.Properties[ lineNumber ].Name );
                            Console.WriteLine(cmd.CurrentFile.Properties[ lineNumber ].Type );
                            Console.WriteLine("MACTH FOUND");
                        }
                        else 
                        {
                            Console.WriteLine("however, no matches found.");
                        }
                        */
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

                        if (line.Contains("NoMetadata"))
                        {
                            cmd.CurrentFile.SClassInfo |= SC.NoMetadata;
                        }
                    }

                    if (line.Contains("class") && cmd.CurrentFile.ClassName == null && LineIsNotComment(line) && ClassIsNotFwd(line))
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
                        classDeclarations += "\tSAT_DECLARE_CLASS(";
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
                }

                sr.Close();
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

            string className = cmd.CurrentFile.ClassName;

            try
            {
                cmd.GeneratedSource.AppendLine(GenWarning);

                cmd.GeneratedSource.AppendLine(string.Format("#include \"{0}\"", string.Format("{0}.Gen.h", className)));

                cmd.GeneratedSource.AppendLine(string.Format("#include \"{0}\"", "Saturn/GameFramework/Core/GameScript.h"));
                cmd.GeneratedSource.AppendLine(string.Format("#include \"{0}\"", "Saturn/GameFramework/Core/ClassMetadataHandler.h"));
                cmd.GeneratedSource.AppendLine(string.Format("#include \"{0}\"", "Saturn/Scene/Entity.h"));

                cmd.GeneratedSource.AppendLine(string.Format("#include \"{0}\"\r\n", string.Format("{0}.h", className)));

                string cExternBeg = "extern \"C\" {\r\n";
                string cExternEnd = "}\r\n";

                cmd.GeneratedSource.AppendLine(cExternBeg);

                // Check if this class is spawnable
                if ((cmd.CurrentFile.SClassInfo & SC.Spawnable) == SC.Spawnable)
                {
                    string function = string.Format("__declspec(dllexport) Saturn::Entity* _Z_Create_{0}(Saturn::Scene* pScene)", className);
                    function += "\r\n{\r\n";
                    function += string.Format("\tSaturn::Ref<{0}> Target = Saturn::Ref<{0}>::Create();\r\n", className);
                    function += "\tSaturn::Ref<Saturn::Entity> TargetReturn = Target.As<Saturn::Entity>();\r\n";
                    function += "\treturn TargetReturn.Get();\r\n";
                    function += "}\r\n";

                    cmd.GeneratedSource.AppendLine(function);

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

                    cmd.GeneratedSource.AppendLine("//^^^ NO Spawnable\r\n");
                }

                cmd.GeneratedSource.AppendLine(cExternEnd);

                if (ProjectInfo.Instance.CurrentConfigKind != ConfigKind.Dist)
                {
                    if ((cmd.CurrentFile.SClassInfo & SC.NoMetadata) != SC.NoMetadata)
                    {
                        string metadata = string.Format("static void ReflCreateMetadataFor_{0}()\r\n", className);
                        metadata += "{\r\n";
                        metadata += string.Format("\tSaturn::SClassMetadata __Metadata_{0};\r\n", className);
                        metadata += string.Format("\t__Metadata_{0}.Name = \"{0}\";\r\n", className);
                        metadata += string.Format("\t__Metadata_{0}.ParentClassName = \"{1}\";\r\n", className, cmd.CurrentFile.BaseClass);
                        metadata += string.Format("\t__Metadata_{0}.GeneratedSourcePath = __FILE__;\r\n", className);
                        metadata += string.Format("\t__Metadata_{0}.HeaderPath = \"{1}\";\r\n", className, HeaderPath.Replace("\\", "\\\\"));
                        metadata += string.Format("\t__Metadata_{0}.ExternalData = true;\r\n", className);
                        metadata += string.Format("\tSaturn::ClassMetadataHandler::Get().Add( __Metadata_{0} );\r\n", className);
                        metadata += "}\r\n";

                        cmd.GeneratedSource.AppendLine(metadata);
                    }
                    else 
                    {
                        string metadata = string.Format("static void ReflCreateMetadataFor_{0}()\r\n", className);
                        metadata += "{\r\n";
                        metadata += string.Format("\tSaturn::SClassMetadata __Metadata_{0};\r\n", className);
                        metadata += string.Format("\t__Metadata_{0}.Name = \"{0}\";\r\n", className);
                        metadata += string.Format("\tSaturn::ClassMetadataHandler::Get().Add( __Metadata_{0} );\r\n", className);
                        metadata += "}\r\n";

                        cmd.GeneratedSource.AppendLine(metadata);
                    }
                }

                // Internal Class
                string internalClassName = string.Format( "{0}Int", className );

                string classStructure = string.Format( "class {0}\r\n", internalClassName);
                classStructure += "{\r\n";
                classStructure += "public:\r\n";

                // Properties
                foreach (var kv in cmd.CurrentFile.Properties) 
                {
                    Property property = kv.Value;

                    classStructure += string.Format( "\tstatic void Set{0}( {1}* pClass, {2} value )\r\n", property.Name, className, property.Type);
                    classStructure += "\t{\r\n";
                    classStructure += string.Format( "\t\tpClass->{0} = value;\r\n", property.Name, property.Type);
                    classStructure += "\t}\r\n";

                    classStructure += string.Format( "\tstatic {0} Get{1}( {2}* pClass )\r\n", property.Type, property.Name, className );
                    classStructure += "\t{\r\n";
                    classStructure += string.Format( "\t\treturn pClass->{0};\r\n", property.Name );
                    classStructure += "\t}\r\n";

                    classStructure += "\r\n";
                }

                classStructure += "};\r\n";
                classStructure += "\r\n";

                classStructure += string.Format( "static void ReflRegisterPropetiesFor_{0}()\r\n", className);
                classStructure += "{\r\n";

                /* AMEND: SProperty when changed.
                struct SProperty
	            {
		            std::string Name;
		            std::string Type;
		            SPropertyFlags Flags;

		            const void* SetPropertyFunction;
		            const void* GetPropertyFunction;
                };*/

                foreach (var kv in cmd.CurrentFile.Properties)
                {
                    Property property = kv.Value;

                    classStructure += string.Format("\t////////////////////////////////////////////////////////////////////////// {0}\r\n", property.Name.ToUpper());

                    string type = "Saturn::SPropertyType::";
                    if( property.Type == "char" ) 
                    {
                        type += "Char";
                    }
                    else if( property.Type == "float" ) 
                    {
                        type += "Float";
                    }
                    else { type += "Float"; }

                    classStructure += string.Format("\tSaturn::SProperty Prop_{0} = {{ .Name = \"{0}\", .Type = {1}, .Flags = Saturn::SPropertyFlags::None }};\r\n", property.Name, type);
                    classStructure += string.Format("\tProp_{1}.pSetPropertyFunction = &{0}::Set{1};\r\n", internalClassName, property.Name);
                    classStructure += string.Format("\tProp_{1}.pGetPropertyFunction = &{0}::Get{1};\r\n", internalClassName, property.Name);

                    classStructure += string.Format("\tSaturn::ClassMetadataHandler::Get().RegisterProperty( \"{0}\", Prop_{1} );\r\n", className, property.Name);
                }
                classStructure += "}\r\n";

                cmd.GeneratedSource.AppendLine( classStructure );

                // Auto-Registration (DLL only).
                if (ProjectInfo.Instance.CurrentConfigKind != ConfigKind.Dist)
                {
                    Random random = new Random();
                    int randomNumber = random.Next();
                        
                    string call = string.Format("struct Ar{0}_RTEditor\r\n", className);
                    call += "{\r\n";
                    call += string.Format("\tAr{0}_RTEditor()\r\n", className);
                    call += "\t{\r\n";
                    call += string.Format("\t\tReflCreateMetadataFor_{0}();\r\n", className);
                    call += string.Format("\t\tReflRegisterPropetiesFor_{0}();\r\n", className);
                    call += "\t}\r\n";
                    call += "};\r\n";
                    call += "\r\n";
                    call += string.Format("static Ar{0}_RTEditor Ar{0}_Runtime_{1};", className, randomNumber);

                    cmd.GeneratedSource.AppendLine(call);
                    cmd.GeneratedSource.AppendLine("//^^^ Auto-Registration\r\n");
                }
                else
                {
                    string call = "// No Auto-Registration.\r\n";
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
            string HeaderFilepath = Path.ChangeExtension(SourcePath, ".h");

            string Filename = Path.GetFileName(SourcePath);

            string GenFilepath = ProjectInfo.Instance.BuildDir;
            GenFilepath = Path.Combine(GenFilepath, Filename);
            GenFilepath = Path.ChangeExtension(GenFilepath, ".Gen.cpp");

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
            string Filename = Path.GetFileName(HeaderFilepath);

            string GenFilepath = ProjectInfo.Instance.BuildDir;
            GenFilepath = Path.Combine(GenFilepath, Filename);
            GenFilepath = Path.ChangeExtension( GenFilepath, ".Gen.h" );

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

            if( !CommandQueue.ContainsKey(HeaderFilepath) )
            {
                Command cmd = new Command
                {
                    Type = CommandType.Header,
                    GenFilepath = GenFilepath
                };

                if( !CommandQueue.ContainsKey( HeaderFilepath ) )
                {
                    // Very bad.
                    try
                    {
                        CommandQueue.Add(HeaderFilepath, cmd);
                    }
                    catch (Exception)
                    {
                    }
                }
            }

            if (!File.Exists(GenFilepath)) 
            {
                File.Create(GenFilepath).Close();
            }

            GenerateHeader( GenFilepath, HeaderFilepath);

            return false;
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
    }
}
