using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BuildTool.Tools
{
    internal static class DirectoryTools
    {
        public static List<string> DirSearch(string sDir)
        {
            List<string> strings = new List<string>();

            try
            {
                foreach (string d in Directory.GetDirectories(sDir))
                {
                    foreach (string f in Directory.GetFiles(d))
                    {
                        Console.WriteLine(f);
                        strings.Add(f);
                    }
                    DirSearch(d);
                }
            }
            catch (System.Exception excpt)
            {
                Console.WriteLine(excpt.Message);
            }

            return strings;
        }

        public static List<string> DirSearch(string sDir, string ext)
        {
            List<string> strings = new List<string>();

            try
            {
                foreach (string f in Directory.GetFiles(sDir))
                {
                    if (Path.GetExtension(f) != ext)
                        continue;

                    strings.Add(f);
                }
            }
            catch (System.Exception excpt)
            {
                Console.WriteLine(excpt.Message);
            }

            return strings;
        }

        public static List<string> DirSearch(string sDir, bool isSourceOnly)
        {
            List<string> strings = new List<string>();

            try
            {
                foreach (string d in Directory.GetDirectories(sDir))
                {
                    if (isSourceOnly)
                        if (!d.EndsWith("src"))
                            continue;

                    foreach (string f in Directory.GetFiles(d))
                    {
                        strings.Add(f);
                    }
                    DirSearch(d, isSourceOnly);
                }
            }
            catch (System.Exception excpt)
            {
                Console.WriteLine(excpt.Message);
            }

            return strings;
        }
    }
}
