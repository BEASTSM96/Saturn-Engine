using System;
using System.IO;
using System.Text.RegularExpressions;

namespace SaturnBuildTool.Tools
{
    internal class HeaderTool
    {
        public struct CurrentGenFile 
        {
            public SC SClassInfo;
            public string ClassName;
            public string BaseClass;
        }

        public HeaderTool() 
        {
        }

        public static readonly HeaderTool Instance = new HeaderTool();

        // This should only be set when parsing the header, as we are only paring the header.
        private CurrentGenFile CurrentFile = new CurrentGenFile();

        private bool IsFilepathGeneratedH(string Filepath) 
        {
            return Filepath.Contains( ".Gen.h" );
        }

        private bool IsFilepathGeneratedS(string Filepath)
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

        private bool GenerateHeader(string GenHeaderPath, string HeaderPath) 
        {
            bool Result = false;

            FileStream fs = new FileStream(GenHeaderPath, FileMode.Open, FileAccess.Write, FileShare.ReadWrite);
            fs.SetLength(0);

            StreamWriter streamWriter = new StreamWriter(fs);

            string GenWarning = "/* Generated code, DO NOT modify! */";

            string FirstBaseClass = GetFirstBaseClass( HeaderPath );
            CurrentFile.BaseClass = FirstBaseClass;

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

                    if (line.Contains("SCLASS")) 
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

                    if ( line.Contains("class") && CurrentFile.ClassName == null) 
                    {
                        CurrentFile.ClassName = GetClassName(line);
                    }

                    if (line.Contains("GENERATED_BODY"))
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
                StreamReader sr = new StreamReader(SourcePath);

                string line;
                int lineNumber = 0;

                streamWriter.WriteLine(GenWarning);

                streamWriter.WriteLine(string.Format("#include \"{0}\"", string.Format( "{0}.Gen.h", CurrentFile.ClassName ) ));

                streamWriter.WriteLine(string.Format("#include \"{0}\"", "Saturn/GameFramework/GameScript.h"));
                streamWriter.WriteLine(string.Format("#include \"{0}\"", "Saturn/GameFramework/ScriptManager.h"));
                streamWriter.WriteLine(string.Format("#include \"{0}\"", "Saturn/Scene/Entity.h"));

                streamWriter.WriteLine(string.Format("#include \"{0}\"\r\n", string.Format( "{0}.h", CurrentFile.ClassName )));

                while ((line = sr.ReadLine()) != null)
                {
                    lineNumber++;

                    // Check if this class is spawnable
                    if ( ( CurrentFile.SClassInfo & SC.Spawnable ) == SC.Spawnable) 
                    {
                        // If our class in spawnable we can now create the spawn function.

                        string cExternBeg = "extern \"C\" {\r\n";
                        string cExternEnd = "}\r\n";

                        streamWriter.WriteLine( cExternBeg );

                        string function = "__declspec(dllexport) Saturn::Entity* _Z_Create_";
                        function += CurrentFile.ClassName;
                        function += "()\r\n";
                        function += "{\r\n";
                        function += "\t return ";
                        function += CurrentFile.ClassName;
                        function += "::Spawn();\r\n";
                        function += "}\r\n";

                        streamWriter.WriteLine( function );

                        string functionBaseClass = "__declspec(dllexport) Saturn::Entity* _Z_Create_";
                        functionBaseClass += CurrentFile.ClassName;
                        functionBaseClass += "_FromBase";
                        functionBaseClass += string.Format( "({0}* Ty)\r\n", CurrentFile.BaseClass );
                        functionBaseClass += "{\r\n";
                        functionBaseClass += "\t return new ";
                        functionBaseClass += CurrentFile.ClassName;
                        functionBaseClass += "(*Ty);\r\n";
                        functionBaseClass += "}\r\n";

                        streamWriter.WriteLine( functionBaseClass );

                        streamWriter.WriteLine( cExternEnd );
                    }

                    if ((CurrentFile.SClassInfo & SC.VisibleInEditor) == SC.VisibleInEditor)
                    {
                        // If our class in spawnable we can now create the spawn function.

                        string func = string.Format("static void _RT_Z_Add{0}ToEditor()\r\n", CurrentFile.ClassName);
                        func += "{\r\n";
                        func += string.Format("\t Saturn::ScriptManager::Get().RT_AddToEditor(\"{0}\");\r\n", CurrentFile.ClassName);
                        func += "}\r\n";

                        string call = string.Format("struct _Z_{0}_RT_Editor\r\n", CurrentFile.ClassName);
                        call += "{\r\n";
                        call += string.Format("\t_Z_{0}_RT_Editor()\r\n", CurrentFile.ClassName);
                        call += "\t{\r\n";
                        call += string.Format("\t\t_RT_Z_Add{0}ToEditor();\r\n", CurrentFile.ClassName);
                        call += "\t}\r\n";
                        call += "};\r\n";
                        call += "\r\n";
                        call += string.Format("static _Z_{0}_RT_Editor _Z_RT_{0};", CurrentFile.ClassName);

                        streamWriter.WriteLine(func);
                        streamWriter.WriteLine(call);
                    }

                    break; // Only do this once
                }

                sr.Close();
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
