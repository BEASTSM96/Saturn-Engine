using System;

using SaturnBuildTool.Tools;

namespace SaturnBuildTool
{
    internal class LinuxToolchain : Toolchain
    {
        public LinuxToolchain(UserTarget target)
        {
            TargetToBuild = target;
        }

        public override int Compile(string InputFile, bool RunHeaderTool = true)
        {
            CompileTask compileTask = new CompileTask(InputFile, TargetToBuild);

            //HeaderTool.Instance.GenerateHeader(InputFile);
            //HeaderTool.Instance.GenerateSource(InputFile);

            int result = -1;

            try
            {
                //result = compileTask.Execute();
            }
            catch (System.Exception excpt)
            {
                Console.WriteLine(excpt.Message);
            }

            return result;
        }

        public override int Link()
        {
            LinkTask link = new LinkTask(TargetToBuild);

            int result = -1;

            try
            {
                //result = link.Execute();
            }
            catch (System.Exception excpt)
            {
                Console.WriteLine(excpt.Message);
            }

            return result;
        }
    }
}
