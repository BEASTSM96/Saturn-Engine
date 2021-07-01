using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Saturn
{

    [StructLayout(LayoutKind.Sequential)]
    public struct Transform
    {
        public Vector3 Position;
        public Vector3 Rotation;
        public Vector3 Scale;

        // If we need these:
        // glm::vec3 right = glm::normalize(glm::rotate(rotation, glm::vec3(1.0F, 0.0F, 0.0F)));
        // glm::vec3 up = glm::normalize(glm::rotate(rotation, glm::vec3(0.0F, 1.0F, 0.0F)));
        // glm::vec3 forward = glm::normalize(glm::rotate(rotation, glm::vec3(0.0F, 0.0F, -1.0F)));
        public Vector3 Up { get; }
        public Vector3 Right { get; }
        public Vector3 Forward { get; }

    }

    public class TransformComponent : Component
    {
        public Transform Transform
        {
            get
            {
                GetTransform_Native(Entity.EntityID, out Transform result);
                return result;
            }

            set
            {
                SetTransform_Native(Entity.EntityID, ref value);
            }
        }

        public Vector3 Translation
        {
            get
            {
                GetTranslation_Native(Entity.EntityID, out Vector3 result);
                return result;
            }

            set
            {
                SetTranslation_Native(Entity.EntityID, ref value);
            }
        }

        public Vector3 Rotation
        {
            get
            {
                GetRotation_Native(Entity.EntityID, out Vector3 result);
                return result;
            }

            set
            {
                SetRotation_Native(Entity.EntityID, ref value);
            }
        }

        public Vector3 Scale
        {
            get
            {
                GetScale_Native(Entity.EntityID, out Vector3 result);
                return result;
            }

            set
            {
                SetScale_Native(Entity.EntityID, ref value);
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetTransform_Native(ulong entityID, out Transform outTransform);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetTransform_Native(ulong entityID, ref Transform inTransform);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetTranslation_Native(ulong entityID, out Vector3 outTranslation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetTranslation_Native(ulong entityID, ref Vector3 inTranslation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetRotation_Native(ulong entityID, out Vector3 outRotation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetRotation_Native(ulong entityID, ref Vector3 inRotation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetScale_Native(ulong entityID, out Vector3 outScale);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetScale_Native(ulong entityID, ref Vector3 inScale);

    }
}
