using System;
using System.Runtime.CompilerServices;

namespace Saturn
{
    public enum ForceType
    {
        Force = 1,
        Acceleration = 2,
        Impulse = 3,
        VelocityChange = 4
    }

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
        public void ApplyForce(Vector3 dir, ForceType type)
        {
            ApplyForce_Native(Entity.EntityID, dir, type);
        }
        public void Rotate(Vector3 rot)
        {
            Rotate_Native(Entity.EntityID, ref rot);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Rotate_Native(ulong entityID, ref Vector3 rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void ApplyForce_Native(ulong entityID, Vector3 dir, ForceType type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetLinearVelocity_Native(ulong entityID, out Vector3 velocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetLinearVelocity_Native(ulong entityID, ref Vector3 velocity);

    }
}
