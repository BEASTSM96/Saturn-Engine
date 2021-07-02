using System;
using Saturn;

namespace ExampleApp
{
    class Null { }

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
        }

        public void OnCollisionBegin()
        {
            Log.Info("OnCollisionBegin");
        }

        public void OnCollisionExit()
        {
            Log.Info("OnCollisionExit");
        }

        public void OnUpdate(float ts)
        {
            if (Input.IsKeyPressed(Key.W))
            {
                Log.Info("W Key Pressed");
            }

            if (Input.IsKeyPressed(Key.A))
            {
                Log.Info("A Key Pressed");
            }

            if (Input.IsKeyPressed(Key.S))
            {
                Log.Info("S Key Pressed");
            }

            if (Input.IsKeyPressed(Key.D))
            {
                Log.Info("D Key Pressed");
            }

            if (Input.IsKeyPressed(Key.X))
            {
            }
        }

    }

    class FreeCamera : Entity
    {
        public float WalkingSpeed = 3.0F;
        public float RunSpeed = 5.0F;
        public float JumpForce = 1.0F;
        public float CameraForwardOffset = 0.2F;
        public float CameraYOffset = 8.850F;

        [NonSerialized]
        public float MouseSensitivity = 2;

        private bool m_Colliding = false;
        private float m_CurrentSpeed;

        private PhysXRigidbodyComponent m_RigidBody;
        private TransformComponent m_Transform;
        private TransformComponent m_CameraTransform;

        private Entity m_CameraEntity;

        private Vector2 m_LastMousePosition;

        private float m_CurrentYMovement = 0.0F;

        private Vector2 m_MovementDirection = new Vector2(0.0F);
        private bool m_ShouldJump = false;
        private Vector3 localPos = new Vector3(0, 0, 0);
        private Vector3 localRotation = new Vector3(0, 0, 0);

        void OnCreate()
        {
            m_Transform = GetComponent<TransformComponent>();
            m_RigidBody = GetComponent<PhysXRigidbodyComponent>();

            m_CameraEntity = FindEntityByTag("Camera");
            m_CameraTransform = m_CameraEntity.GetComponent<TransformComponent>();

            m_CurrentSpeed = WalkingSpeed;

            m_LastMousePosition = Input.GetMousePosition();

            Input.SetCursorMode(CursorMode.Locked);
        }

        void OnUpdate(float ts)
        {
            // Check Mouse
            {
                if (Input.IsKeyPressed(Key.Escape) && Input.GetCursorMode() == CursorMode.Locked)
                    Input.SetCursorMode(CursorMode.Normal);

                if (Input.IsMouseButtonPressed(MouseButton.Left) && Input.GetCursorMode() == CursorMode.Normal)
                    Input.SetCursorMode(CursorMode.Locked);
            }

            // Update Location

            if (Input.IsKeyPressed(Key.W))
                localPos.Y += 1.0f;

            if(Input.IsKeyPressed(Key.S))
                localPos.Y -= 1.0f;

            if (Input.IsKeyPressed(Key.D))
                localPos.X += 1.0f;

            if (Input.IsKeyPressed(Key.A))
                localPos.X -= 1.0f;

            m_CurrentSpeed = Input.IsKeyPressed(Key.LeftControl) ? RunSpeed : WalkingSpeed;

            // Update Movement
            {
                m_Transform.Translation = localPos;
            }

            // Update Rotation
            {
               float mouseX = Input.GetMousePosition().X;
               float mouseY = Input.GetMousePosition().Y;

                Vector2 currentMousePosition = Input.GetMousePosition();
                Vector2 delta = m_LastMousePosition - currentMousePosition;
                float xRotation = delta.Y * (MouseSensitivity * 0.05F) * ts;

                if (xRotation != 0.0F)
                {
                 //   m_Transform.Rotation += new Vector3(xRotation, 0.0F, 0.0F);
                }

                localRotation.X = mouseX;
                localRotation.Y = mouseY;

                Log.Info(mouseX.ToString());
                Log.Info("local pos" + localRotation.X.ToString());

                m_Transform.Rotation = new Vector3(Mathf.Clamp(mouseX * Mathf.Rad2Deg, -80.0F, 80.0F), 0.0F, 0.0F) * Mathf.Deg2Rad;
                m_CameraTransform.Rotation = new Vector3(Mathf.Clamp(mouseX * Mathf.Rad2Deg, -80.0F, 80.0F), 0.0F, 0.0F) * Mathf.Deg2Rad;
            }

            UpdateMovementInput();
            UpdateRotation(ts);
            UpdateCameraTransform(ts);
            UpdateMovement();
        }

        private void UpdateMovementInput()
        {
        }

        private void UpdateRotation(float ts)
        {
        }

        private void UpdateMovement()
        {
        }

        private void UpdateCameraTransform(float ts)
        {
        }
    }
}