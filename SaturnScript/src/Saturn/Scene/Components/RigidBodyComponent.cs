using System;
using System.Runtime.CompilerServices;

namespace Saturn
{
    public class PhysXRigidbodyComponent : Component
    {
        public enum Type
        {
            Static,
            Dynamic
        }

        public Vector3 GetLinearVelocity()
        {
            GetLinearVelocity_Native(Entity.EntityID, out Vector3 velocity);
            return velocity;
        }

        public void SetLinearVelocity(Vector3 velocity)
        {
            SetLinearVelocity_Native(Entity.EntityID, ref velocity);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetLinearVelocity_Native(ulong entityID, out Vector3 velocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetLinearVelocity_Native(ulong entityID, ref Vector3 velocity);

    }
}
