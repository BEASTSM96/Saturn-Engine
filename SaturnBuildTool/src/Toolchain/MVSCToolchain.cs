using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;
using System.IO;

using BuildTool.Tools;
using System.Xml;

namespace BuildTool.Toolchain
{
    internal class MVSCToolchain
    {
        private UserTarget TargetToBuild;

        public MVSCToolchain(UserTarget target)
        {
            TargetToBuild = target;
        }

        public int Compile(string InputFile)
        {
            CompileTask compileTask = new CompileTask( InputFile, TargetToBuild );

            HeaderTool.Instance.GenerateHeader( InputFile );
            //HeaderTool.Instance.GenerateSource( InputFile );

            int result = -1;

            try
            {
                result = compileTask.Execute();
            }
            catch(System.Exception excpt)
            {
                Console.WriteLine(excpt.Message);
            }

            return result;
        }

        public int Link()
        {
            LinkTask link = new LinkTask(TargetToBuild);

            int result = -1;

            try
            {
                result = link.Execute();
            }
            catch (System.Exception excpt)
            {
                Console.WriteLine(excpt.Message);
            }

            return result;
        }
    }
}
