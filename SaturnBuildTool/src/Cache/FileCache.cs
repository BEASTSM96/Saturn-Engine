using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace SaturnBuildTool.Cache
{
    internal class FileCache
    {
        public IDictionary<string, DateTime> FilesToCache { get; set; }
        public IDictionary<string, DateTime> FilesInCache { get; set; }

        private string Filepath;

        public FileCache(string CacheLocation)
        {
            FilesToCache = new Dictionary<string, DateTime>();
            FilesInCache = new Dictionary<string, DateTime>();

            Filepath = CacheLocation;
        }

        public FileCache()
        {
            FilesToCache = new Dictionary<string, DateTime>();
            FilesInCache = new Dictionary<string, DateTime>();

            Filepath = "";
        }

        public void CacheFile(string Filepath)
        {
            if (IsCppFile(Filepath)) 
            {
                FilesToCache.Add(Filepath, File.GetLastWriteTime(Filepath));
            }
        }

        public bool IsCppFile( string Filepath )
        {
            return Path.GetExtension(Filepath) == ".cpp" || Path.GetExtension(Filepath) == ".h";
        }

        public bool IsSourceFile(string Filepath)
        {
            return Path.GetExtension(Filepath) == ".cpp";
        }

        public bool IsFileInCache(string Filepath) 
        {
            return FilesToCache.ContainsKey(Filepath);
        }

        public static void RT_WriteCache( FileCache fileCache ) 
        {
            foreach (KeyValuePair<string, DateTime> kv in fileCache.FilesToCache) 
            {
                DateTime time;
                fileCache.FilesInCache.TryGetValue(kv.Key, out time);

                // Has the file been updated?
                if(time != kv.Value)
                {
                    // Yes, lets try to add it in the cache

                    if(fileCache.FilesInCache.ContainsKey(kv.Key))
                    {
                        fileCache.FilesInCache[kv.Key] = kv.Value;
                    }
                    else
                    {
                        fileCache.FilesInCache.Add(kv);
                    }
                }
            }
            
            fileCache.FilesToCache.Clear();

            // HACK: Clear the file.
            File.WriteAllText(fileCache.Filepath, string.Empty);

            FileStream fs = new FileStream(fileCache.Filepath, FileMode.Append, FileAccess.Write, FileShare.ReadWrite);

            StreamWriter sw = new StreamWriter( fs );

            // I'm not sure if we need the new line.
            byte[] newLine = Encoding.ASCII.GetBytes("\r\n");
            byte[] fileHeader = Encoding.ASCII.GetBytes(".FC");

            fs.Write(fileHeader, 0, fileHeader.Length);
            fs.Write(newLine, 0, newLine.Length);

            foreach (KeyValuePair<string, DateTime> kv in fileCache.FilesInCache) 
            {
                string format = kv.Key + ":" + kv.Value.ToBinary();

                byte[] bytes = Encoding.ASCII.GetBytes(format);

                fs.Write(bytes, 0, bytes.Length);
                fs.Write(newLine, 0, newLine.Length);
            }

            sw.Close();
            fs.Close();
        }

        public static FileCache Load( string FileCachePath ) 
        {
            FileCache fc = new FileCache(FileCachePath);
            fc.Filepath = FileCachePath;

            if (!File.Exists(FileCachePath))
                File.Create( FileCachePath );

            FileStream fs = new FileStream(FileCachePath, FileMode.Open);

            byte[] headerBytes = new byte[3];
            fs.Read(headerBytes, 0, headerBytes.Length);

            string fileHeader = Encoding.ASCII.GetString(headerBytes);

            if( fileHeader != ".FC" )
            {
                Console.WriteLine("File cache file has an invalid header.");
                fs.Close();

                return null;
            }

            long remainingBytes = fs.Length - headerBytes.Length;

            byte[] bytes = new byte[remainingBytes];

            fs.Read(bytes, 0, bytes.Length);

            string data = Encoding.ASCII.GetString( bytes );

            string[] entries = data.Split(new char[] { '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries);
            foreach ( string entry in entries )
            {
                int colonIndex = entry.LastIndexOf(':');

                if (colonIndex == -1) 
                {
                    continue;
                }

                string filePath = entry.Substring(0, colonIndex);
                string time = entry.Substring(colonIndex + 1);

                if (long.TryParse(time, out long writeTime))
                {
                    DateTime dt = DateTime.FromBinary(writeTime);
                    fc.FilesInCache.Add(filePath, dt);
                }
                else
                {
                    Console.WriteLine("Invalid date format.");
                }
            }

            fs.Close();
            return fc;
        }
    }
}
