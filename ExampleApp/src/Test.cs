﻿using System;
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
            Log.Info("OnCreate");
        }

        public void OnBeginPlay()
        {
            Log.Info("OnBeginPlay");
            CreateComponent<MeshComponent>();
            GetComponent<MeshComponent>();
        }

        public void OnUpdate(float ts)
        {
            if (Input.IsKeyPressed((UInt16)Input.Key.W))
            {
                Log.Info("W Key Pressed");
            }

            if (Input.IsKeyPressed((UInt16)Input.Key.A))
            {
                Log.Info("A Key Pressed");
            }

            if (Input.IsKeyPressed((UInt16)Input.Key.S))
            {
                Log.Info("S Key Pressed");
            }

            if (Input.IsKeyPressed((UInt16)Input.Key.D))
            {
                Log.Info("D Key Pressed");
            }
        }

    }

    class TestII
    {
       
    }
}
