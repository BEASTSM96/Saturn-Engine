using System;
using System.Runtime.CompilerServices;

namespace Saturn
{
    public class TagComponent :  Component
    {
        public string Tag 
        {
            get => GetTag_Native(Entity.EntityID);
            set => SetTag_Native(Entity.EntityID, value);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetTag_Native(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string SetTag_Native(ulong entityID, string tag);
    }
}
