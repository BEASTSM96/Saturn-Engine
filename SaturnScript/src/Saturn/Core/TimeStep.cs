using System;
using System.Runtime.CompilerServices;

namespace Saturn
{
    public static class TimeStep
    {
        public static float GetTimeStep()
        {
            GetTimeStep_Native(out float time);
            return time;
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetTimeStep_Native(out float timestep);
    }
}
