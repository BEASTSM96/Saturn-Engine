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

        public static void Error(string msg)
        {
            Error_Native(msg);
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Error_Native(string message);

        public static void Warn(string msg)
        {
            Warn_Native(msg);
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Warn_Native(string message);

        public static void Info(string msg)
        {
            Info_Native(msg);
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Info_Native(string message);

        public static void Trace(string msg)
        {
            Trace_Native(msg);
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Trace_Native(string message);
    }
}
