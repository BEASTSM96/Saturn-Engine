using System;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;

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
            CurrentFile.Properties = new Dictionary<int, Property>();
        }

        public static readonly HeaderTool Instance = new HeaderTool();

        // If the last line had a SPROPERTY macro.
        public bool LastLineHadSP;

        // This should only be set when parsing the header, as we are only paring the header.
        private CurrentGenFile CurrentFile = new CurrentGenFile();

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
                            continue;

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

            if(!AnyReflectiveCodeFound)
                Console.WriteLine("No reflective code was found for " + Path.GetFileName(HeaderPath) );

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
                            continue;

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

            StreamWriter streamWriter = new StreamWriter(fs);

            string GenWarning = "/* Generated code, DO NOT modify! */";

            // Reset.
            string FirstBaseClass = GetFirstBaseClass( HeaderPath );
            CurrentFile.BaseClass = FirstBaseClass;

            // Reset the class name.
            CurrentFile.ClassName = null;

            Console.WriteLine( FirstBaseClass );

            try
            {
                StreamReader sr = new StreamReader(HeaderPath);

                string line;
                int lineNumber = 0;

                streamWriter.WriteLine(GenWarning);

                streamWriter.WriteLine("#pragma once\r\n");

                streamWriter.WriteLine(string.Format("#include \"{0}\"", "Saturn/GameFramework/GameScript.h"));

                streamWriter.WriteLine("\r\n");

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

                            CurrentFile.Properties[lineNumber] = p;
                        }

                        LastLineHadSP = false;
                    }

                    if (line.Contains("SCLASS") && LineIsNotComment( line ) ) 
                    {
                        if (line.Contains("Spawnable"))
                        {
                            CurrentFile.SClassInfo |= SC.Spawnable;
                        }

                        if (line.Contains("VisibleInEditor"))
                        {
                            CurrentFile.SClassInfo |= SC.VisibleInEditor;
                        }
                    }

                    if( line.Contains("class") && CurrentFile.ClassName == null && LineIsNotComment( line ) ) 
                    {
                        CurrentFile.ClassName = GetClassName(line);
                        Console.WriteLine(CurrentFile.ClassName);
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

                        streamWriter.WriteLine("\r\n");

                        streamWriter.WriteLine(CFI);

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

                        streamWriter.WriteLine("\r\n");
                        streamWriter.WriteLine(classDeclarations);

                        idGeneratedBody += baseFileId;
                        idGeneratedBody += "_CLASSDECLS \r\n";

                        streamWriter.WriteLine("\r\n");
                        streamWriter.WriteLine(idGeneratedBody);
                    }

                    if (line.Contains("SPROPERTY") && LineIsNotComment(line))
                    {
                        LastLineHadSP = true;

                        Property p = new Property
                        {
                            Line = lineNumber + 1
                        };

                        CurrentFile.Properties.Add(p.Line, p);

                        if (line.Contains("VisibleInEditor"))
                        {
                            p.Flags |= SP.VisibleInEditor;
                        }
                    }


                }

                sr.Close();
            }
            catch (Exception e)
            {
                Console.WriteLine("The file could not be read:");
                Console.WriteLine(e.Message);
            }

            streamWriter.Close();

            return Result;
        }

        private bool GenerateSource(string GenSourcePath, string SourcePath) 
        {
            bool Result = false;

            FileStream fs = new FileStream(GenSourcePath, FileMode.Open, FileAccess.Write, FileShare.ReadWrite);
            fs.SetLength(0);

            StreamWriter streamWriter = new StreamWriter(fs);

            string GenWarning = "/* Generated code, DO NOT modify! */";

            try
            {
                streamWriter.WriteLine(GenWarning);

                streamWriter.WriteLine(string.Format("#include \"{0}\"", string.Format( "{0}.Gen.h", CurrentFile.ClassName ) ));

                streamWriter.WriteLine(string.Format("#include \"{0}\"", "Saturn/GameFramework/GameScript.h"));
                streamWriter.WriteLine(string.Format("#include \"{0}\"", "Saturn/GameFramework/GamePrefabList.h"));
                streamWriter.WriteLine(string.Format("#include \"{0}\"", "Saturn/GameFramework/EntityScriptManager.h"));
                streamWriter.WriteLine(string.Format("#include \"{0}\"", "Saturn/Scene/Entity.h"));

                streamWriter.WriteLine(string.Format("#include \"{0}\"\r\n", string.Format( "{0}.h", CurrentFile.ClassName )));

                // Check if this class is spawnable
                if ((CurrentFile.SClassInfo & SC.Spawnable) == SC.Spawnable)
                {
                    // If our class in spawnable we can now create the spawn function.

                    string cExternBeg = "extern \"C\" {\r\n";
                    string cExternEnd = "}\r\n";

                    streamWriter.WriteLine(cExternBeg);

                    string function = "__declspec(dllexport) Saturn::Entity* _Z_Create_";
                    function += CurrentFile.ClassName;
                    function += "()\r\n";
                    function += "{\r\n";
                    function += "\t return ";
                    function += CurrentFile.ClassName;
                    function += "::Spawn();\r\n";
                    function += "}\r\n";

                    streamWriter.WriteLine(function);

                    string functionBaseClass = "__declspec(dllexport) Saturn::Entity* _Z_Create_";
                    functionBaseClass += CurrentFile.ClassName;
                    functionBaseClass += "_FromBase";
                    functionBaseClass += string.Format("({0}* Ty)\r\n", CurrentFile.BaseClass);
                    functionBaseClass += "{\r\n";
                    functionBaseClass += "\t return new ";
                    functionBaseClass += CurrentFile.ClassName;
                    functionBaseClass += "(*Ty);\r\n";
                    functionBaseClass += "}\r\n";

                    streamWriter.WriteLine(functionBaseClass);

                    streamWriter.WriteLine(cExternEnd);

                    streamWriter.WriteLine("//^^^ Spawnable\r\n");
                }
                else 
                {
                    string cExternBeg = "extern \"C\" {\r\n";
                    string cExternEnd = "}\r\n";

                    streamWriter.WriteLine(cExternBeg);

                    string function = "__declspec(dllexport) Saturn::Entity* _Z_Create_";
                    function += CurrentFile.ClassName;
                    function += "()\r\n";
                    function += "{\r\n";
                    function += "\treturn nullptr;\r\n";
                    function += "}\r\n";

                    streamWriter.WriteLine(function);

                    string functionBaseClass = "__declspec(dllexport) Saturn::Entity* _Z_Create_";
                    functionBaseClass += CurrentFile.ClassName;
                    functionBaseClass += "_FromBase";
                    functionBaseClass += string.Format("({0}* Ty)\r\n", CurrentFile.BaseClass);
                    functionBaseClass += "{\r\n";
                    functionBaseClass += "\treturn nullptr;\r\n";
                    functionBaseClass += "}\r\n";

                    streamWriter.WriteLine(functionBaseClass);

                    streamWriter.WriteLine(cExternEnd);

                    streamWriter.WriteLine("//^^^ NO Spawnable\r\n");
                }

                if ((CurrentFile.SClassInfo & SC.VisibleInEditor) == SC.VisibleInEditor)
                {
                    // If our class in spawnable we can now create the spawn function.

                    string vfunc = string.Format("static void _RT_Z_Add{0}ToEditor()\r\n", CurrentFile.ClassName);
                    vfunc += "{\r\n";
                    vfunc += string.Format("\t Saturn::EntityScriptManager::Get().RT_AddToEditor(\"{0}\");\r\n", CurrentFile.ClassName);
                    vfunc += "}\r\n";
                    vfunc += "//^^^ VisibleInEditor\r\n";

                    streamWriter.WriteLine(vfunc);
                }
                else
                {
                    string vfunc = string.Format("static void _RT_Z_Add{0}ToEditor()\r\n", CurrentFile.ClassName);
                    vfunc += "{\r\n";
                    vfunc += "}\r\n";
                    vfunc += "//^^^ NO VisibleInEditor\r\n";

                    streamWriter.WriteLine(vfunc);
                }

                // Add to prefab list
                string prefab = string.Format("static void _RT_Z_AddPrefab_{0}()\r\n", CurrentFile.ClassName);
                prefab += "{\r\n";
                prefab += string.Format("\t Saturn::GamePrefabList::Get().Add(\"{0}\");\r\n", CurrentFile.ClassName);
                prefab += "}\r\n";
                prefab += "//^^^ Type is prefab...\r\n";

                streamWriter.WriteLine(prefab);

                // Properties
                string propfunc = string.Format("static void _Zp_{0}_Reg_Props()\r\n", CurrentFile.ClassName);
                propfunc += "{\r\n";

                foreach (KeyValuePair<int, Property> kv in CurrentFile.Properties)
                {
                    int ln = kv.Key;
                    Property property = kv.Value;

                    propfunc += string.Format("\t// [_Zp_Public_Prop] {0} of type {1}\r\n", property.Name.ToUpper(), property.Type.ToUpper());
                    propfunc += string.Format("\t//Saturn::EntityScriptManager::Get().AddProperty( \"{0}\", \"{1}\", (void*)&{2}::{1});\r\n", CurrentFile.ClassName, property.Name, CurrentFile.ClassName);
                }

                propfunc += "}\r\n";
                propfunc += "//^^^ Public Properties\r\n";

                streamWriter.WriteLine(propfunc);

                // Auto-Registration

                Random random = new Random();
                int randomNumber = random.Next();

                string call = string.Format("struct _Z_{0}_RT_Editor\r\n", CurrentFile.ClassName);
                call += "{\r\n";
                call += string.Format("\t_Z_{0}_RT_Editor()\r\n", CurrentFile.ClassName);
                call += "\t{\r\n";
                call += string.Format("\t\t_RT_Z_Add{0}ToEditor();\r\n", CurrentFile.ClassName);
                call += string.Format("\t\t_RT_Z_AddPrefab_{0}();\r\n", CurrentFile.ClassName);
                call += string.Format("\t\t_Zp_{0}_Reg_Props();\r\n", CurrentFile.ClassName);
                call += "\r\n";
                call += "\t}\r\n";
                call += "};\r\n";
                call += "\r\n";
                call += string.Format("static _Z_{0}_RT_Editor _Z_RT_{0}_{1};", CurrentFile.ClassName, randomNumber);

                streamWriter.WriteLine(call);
                streamWriter.WriteLine("//^^^ Auto-Registration\r\n");

            }
            catch (Exception e) 
            {
                Console.WriteLine("The file could not be read:");
                Console.WriteLine(e.Message);
            }

            streamWriter.Close();
            fs.Close();

            return Result;
        }

        public bool GenerateSource(string SourcePath)
        {
            string GenFilepath = Path.ChangeExtension(SourcePath, ".Gen.cpp");

            string HeaderFilepath = Path.ChangeExtension(SourcePath, ".h");

            if (!File.Exists(SourcePath))
                return false;

            if (IsFilepathGeneratedS(SourcePath))
                return false;

            // If the header can be reflected so can the source.
            if (!CanFileBeReflected(HeaderFilepath))
                return false;

            if (NoReflectInFile(SourcePath))
                return false;

            // Safe to create the file.
            if (!File.Exists(GenFilepath))
                  File.Create(GenFilepath);

            bool Result = false;

            try
            {
                // We want to read the header.
                Result = GenerateSource(GenFilepath, HeaderFilepath);
            }
            catch (Exception e)
            {
                Console.WriteLine("The file could not be read:");
                Console.WriteLine(e.Message);
            }

            return Result;
        }

        public bool GenerateHeader(string Filepath) 
        {
            // Assume the Filepath is a source file
            string HeaderFilepath = Path.ChangeExtension(Filepath, ".h");

            string GenFilepath = Path.ChangeExtension(HeaderFilepath, ".Gen.h");

            if (!File.Exists(HeaderFilepath))
                return false;

            if (IsFilepathGeneratedH(HeaderFilepath))
                return false;

            if (!CanFileBeReflected(HeaderFilepath))
                return false;

            if (NoReflectInFile(HeaderFilepath))
                return false;

            // Safe to create the file.
            if (!File.Exists(GenFilepath))
                File.Create(GenFilepath);

            bool Result = false;

            try
            {
                Result = GenerateHeader( GenFilepath, HeaderFilepath );
            }
            catch (Exception e)
            {
                Console.WriteLine("The file could not be read:");
                Console.WriteLine(e.Message);
            }

            return Result;
        }
    }
}
