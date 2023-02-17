using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;

namespace SaturnBuildTool.Cache
{
    internal class FileCache
    {
        public IDictionary<string, DateTime> FilesToCache { get; set; }
        public IDictionary<string, DateTime> FilesInCache { get; set; }

        private string m_Location;
        public FileCache(string CacheLocation)
        {
            FilesToCache = new Dictionary<string, DateTime>();
            FilesInCache = new Dictionary<string, DateTime>();

            m_Location = CacheLocation;
        }

        public FileCache()
        {
            FilesToCache = new Dictionary<string, DateTime>();
            FilesInCache = new Dictionary<string, DateTime>();

            m_Location = "";
        }

        public void CacheFile(string Filepath)
        {
            if( IsCppFile(Filepath) )
                FilesToCache.Add(Filepath, File.GetLastWriteTime(Filepath));
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

            var serializer = new SerializerBuilder()
                .WithNamingConvention(PascalCaseNamingConvention.Instance)
                .Build();

            // HACK: Clear the file.
            File.WriteAllText(fileCache.m_Location, string.Empty);

            FileStream fs = new FileStream(fileCache.m_Location, FileMode.Append, FileAccess.Write, FileShare.ReadWrite);

            StreamWriter sw = new StreamWriter( fs );

            var yaml = serializer.Serialize( fileCache );

            sw.WriteLine( yaml );

            sw.Close();
            fs.Close();
        }

        public static FileCache Load( string FileCachePath ) 
        {
            FileCache fc = new FileCache(FileCachePath);
            fc.m_Location = FileCachePath;

            if (!File.Exists(FileCachePath))
                File.Create( FileCachePath );

            StreamReader textReader = new StreamReader( FileCachePath );

            var deserializer = new DeserializerBuilder()
                .WithNamingConvention(PascalCaseNamingConvention.Instance)
                .Build();

            var yaml = deserializer.Deserialize<FileCache>(textReader);


            if (yaml != null)
            {
                yaml.m_Location = FileCachePath;
                fc.FilesToCache = yaml.FilesToCache;
            }
            else 
            {
                textReader.Close();
                return fc;
            }

            textReader.Close();
            return yaml;
        }
    }
}
