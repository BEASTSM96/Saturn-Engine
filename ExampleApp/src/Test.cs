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

    class FPSPlayer : Entity
    {
        public float WalkingSpeed = 10.0F;
        public float RunSpeed = 20.0F;
        public float JumpForce = 50.0F;
        public float CameraForwardOffset = 0.2F;
        public float CameraYOffset = 0.85F;

        [NonSerialized]
        public float MouseSensitivity = 10.0F;

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

        void OnCreate()
        {
            m_Transform = GetComponent<TransformComponent>();
            m_RigidBody = GetComponent<PhysXRigidbodyComponent>();

            m_CurrentSpeed = WalkingSpeed;

            m_CameraEntity = FindEntityByTag("Camera");
            m_CameraTransform = m_CameraEntity.GetComponent<TransformComponent>();

            m_LastMousePosition = Input.GetMousePosition();

            Input.SetCursorMode(CursorMode.Locked);
        }

        void OnUpdate(float ts)
        {
            if (Input.IsKeyPressed(Key.Escape) && Input.GetCursorMode() == CursorMode.Locked)
                Input.SetCursorMode(CursorMode.Normal);

            if (Input.IsMouseButtonPressed(MouseButton.Left) && Input.GetCursorMode() == CursorMode.Normal)
                Input.SetCursorMode(CursorMode.Locked);

            m_CurrentSpeed = Input.IsKeyPressed(Key.LeftControl) ? RunSpeed : WalkingSpeed;

            UpdateMovementInput();
            UpdateRotation(ts);
            UpdateCameraTransform();
            UpdateMovement();
        }

        private void UpdateMovementInput()
        {
            if (Input.IsKeyPressed(Key.W))
                m_MovementDirection.Y = 1.0F;
            else if (Input.IsKeyPressed(Key.S))
                m_MovementDirection.Y = -1.0F;
            else
                m_MovementDirection.Y = 0.0F;

            if (Input.IsKeyPressed(Key.A))
                m_MovementDirection.X = -1.0F;
            else if (Input.IsKeyPressed(Key.D))
                m_MovementDirection.X = 1.0F;
            else
                m_MovementDirection.X = 0.0F;

            m_ShouldJump = Input.IsKeyPressed(Key.Space) && !m_ShouldJump;
        }

        private void UpdateRotation(float ts)
        {
            if (Input.GetCursorMode() != CursorMode.Locked)
                return;

            Vector2 currentMousePosition = Input.GetMousePosition();
            Vector2 delta = m_LastMousePosition - currentMousePosition;
            m_CurrentYMovement = delta.X * MouseSensitivity * ts;
            float xRotation = delta.Y * (MouseSensitivity * 0.05F) * ts;

            if (xRotation != 0.0F)
            {
                m_CameraTransform.Rotation += new Vector3(xRotation, 0.0F, 0.0F);
            }

            m_CameraTransform.Rotation = new Vector3(Mathf.Clamp(m_CameraTransform.Rotation.X * Mathf.Rad2Deg, -80.0F, 80.0F), 0.0F, 0.0F) * Mathf.Deg2Rad;
            m_LastMousePosition = currentMousePosition;

        }

        private void UpdateMovement()
        {
            Vector3 movement = m_CameraTransform.Transform.Right * m_MovementDirection.X + m_CameraTransform.Transform.Forward * m_MovementDirection.Y;
            movement.Normalize();
            Vector3 velocity = movement * m_CurrentSpeed;
            velocity.Y = m_RigidBody.GetLinearVelocity().Y;
            m_RigidBody.SetLinearVelocity(velocity);

            if (m_ShouldJump && m_Colliding)
            {
                //m_RigidBody.AddForce(Vector3.Up * JumpForce, ForceMode.Impulse);
                m_ShouldJump = false;
            }
        }

        private void UpdateCameraTransform()
        {
            Vector3 position = m_Transform.Translation + m_Transform.Transform.Forward * CameraForwardOffset;
            position.Y = m_Transform.Translation.Y + CameraYOffset;
            m_CameraTransform.Translation = position;
            m_CameraTransform.Rotation = new Vector3(m_CameraTransform.Rotation.X, m_Transform.Rotation.Y, m_CameraTransform.Rotation.Z);
        }
    }

    class Null {}
}