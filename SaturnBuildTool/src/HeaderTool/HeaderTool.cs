using System;
using System.IO;
using System.Text.RegularExpressions;

namespace SaturnBuildTool.Tools
{
    internal class HeaderTool
    {
        public HeaderTool() 
        {
        }

        public static readonly HeaderTool Instance = new HeaderTool();

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

            try
            {
                StreamReader sr = new StreamReader(HeaderPath);

                string line;
                int lineNumber = 0;

                streamWriter.WriteLine(GenWarning);

                streamWriter.WriteLine("#pragma once\r\n");

                streamWriter.WriteLine(string.Format("#include \"{0}\"", "Macros.h"));

                while ((line = sr.ReadLine()) != null)
                {
                    lineNumber++;

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

                        break;
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
            //if (!File.Exists(GenFilepath))
            //      File.Create(GenFilepath);

            bool Result = false;

            try
            {
                Result = GenerateSource(GenFilepath, SourcePath);
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
