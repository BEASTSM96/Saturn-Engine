using System;
using System.Runtime.CompilerServices;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Saturn 
{
    public class Entity
    {
        public ulong EntityID { get; private set; }

        protected Entity() { EntityID = 0; }
        internal Entity(ulong id) 
        {
            EntityID = id;
            Console.WriteLine("Entity created {0}", id);
        }

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
        private static extern void CreateComponent_Native( ulong entityID, Type type );

        public bool HasComponent<T>() where T : Component, new()
        {
            return HasComponent_Native(EntityID, typeof(T));
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool HasComponent_Native(ulong entityID, Type type);

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

        public Entity FindEntityByTag(string tag)
        {
            ulong entityID = FindEntityByTag_Native(tag);
            return new Entity(entityID);
        }

        public Entity FindEntityByID(ulong entityID)
        {
            // TODO: Verify the entity id
            return new Entity(entityID);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ulong FindEntityByTag_Native(string tag);

    }
}
