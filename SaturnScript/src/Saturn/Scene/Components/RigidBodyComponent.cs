using System;
using System.Runtime.CompilerServices;

namespace Saturn
{
    public class PhysXRigidbodyComponent : Component
    {
        public void ApplyForce(Vector3 forcedire, PhysXForceMode physXForceMode)
        {
            Console.WriteLine("RB ID {0}", Entity.EntityID);
            AddForce_Native(Entity.EntityID, forcedire, physXForceMode);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void AddForce_Native(ulong entityID, Vector3 forcedire, PhysXForceMode physXForceMode);
    }
}
