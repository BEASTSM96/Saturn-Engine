using BuildTool;
using System;

public class GameTarget : GameUserTarget
{
    public override void Init()
    {
        base.Init();

        //
        //PreprocessorDefines.Add("DEBUG");
        //PreprocessorDefines.Add("BUILD_TOOL_DEBUG");
    }
}