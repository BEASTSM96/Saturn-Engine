using System;
using Saturn;

namespace ExampleApp
{
    class Test : Entity
    {
        public float TestFloat = 0.0f;
        public int TestInt = 0;
        public Vector2 TestVector2;
        public Vector3 TestVector3;
        public Vector4 TestVector4;

        public void OnCreate()
        {
            Log.Fatal("OnCreate");
        }

        public void OnBeginPlay()
        {
            Log.Fatal("OnBeginPlay");
        }

        public void OnUpdate(float ts)
        {
            Log.Fatal("OnUpdate");
        }

    }
}
