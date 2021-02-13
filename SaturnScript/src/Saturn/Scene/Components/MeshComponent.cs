using System;
using System.Runtime.CompilerServices;

namespace Saturn
{
    public class MeshComponent : Component
    {
        public Mesh GetMesh()
        {
            GetMesh_Native(Entity.EntityID, out Mesh mesh);
            return mesh;
        }

        public void SetMesh(Mesh mesh) 
        {
            SetMesh_Native(Entity.EntityID, ref mesh);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetMesh_Native(ulong entityID, out Mesh mesh);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetMesh_Native(ulong entityID, ref Mesh mesh);
    }
}
