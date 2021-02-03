using System;
using System.Runtime.CompilerServices;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Saturn {
    public class Entity
    {
        public uint EntityID { get; private set; }

        ~Entity() 
        {
            Console.WriteLine("Entity destroyed {0}", EntityID);
        }

        public T CreateComponent<T>() where T : Component, new() 
        {
            CreateComponent_Native(EntityID, typeof(T));
            T comp = new T();
            comp.Entity = this;
            return comp;
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CreateComponent_Native( uint entityID, Type type );

        public bool HasComponent<T>() where T : Component, new()
        {
            return HasComponent_Native(EntityID, typeof(T));
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool HasComponent_Native(uint entityID, Type type);

        public T GetComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>()) 
            {
                T comp = new T();
                comp.Entity = this;
                return comp;
            }
            return null;
        }

    }
}
