﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;

namespace BuildTool.Cache
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
            FilesToCache.Add( Filepath, File.GetLastWriteTime( Filepath ) );
        }

        public bool IsFileInCache(string Filepath) { return FilesToCache.ContainsKey(Filepath); }

        public static void RT_WriteCache( FileCache fileCache ) 
        {
            foreach (KeyValuePair<string, DateTime> file in fileCache.FilesToCache) 
            {
                if(!fileCache.FilesInCache.ContainsKey( file.Key ))
                {
                    if (Path.GetExtension(file.Key) != ".cpp" || Path.GetExtension(file.Key) != ".h")
                        continue;

                    fileCache.FilesInCache.Add( file );
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
