using System;
using System.Runtime.CompilerServices;

namespace Saturn 
{ 
    public class Log
    {
        public static void Fatal(string msg)
        {
            Fatal_Native(msg);
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Fatal_Native(string message);
    }
}
