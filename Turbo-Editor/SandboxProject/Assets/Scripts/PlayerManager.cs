using Turbo;

namespace Sandbox
{
	public class PlayerManager : Entity
	{
		float m_Speed = 10.0f;

		Entity m_Camera;

		public float MouseSensitivity;

		private RigidbodyComponent m_Rigidbody;

		private Vector2 m_MovementDirection = Vector2.Zero;
		private Vector2 m_LastMousePosition = Vector2.Zero;

		protected override void OnCreate()
		{
			Input.SetCursorMode(CursorMode.Locked);

			m_Camera = FindEntityByName("Camera");

			m_Rigidbody = GetComponent<RigidbodyComponent>();

			Log.Info("Hello entity!");

			m_LastMousePosition = Input.GetMousePosition();
		}

		protected override void OnUpdate()
		{
			m_MovementDirection = Vector2.Zero;

			Vector3 forward = new Quaternion(m_Camera.Transform.Rotation) * Vector3.Forward;
			Vector3 right = new Quaternion(m_Camera.Transform.Rotation) * Vector3.Right;

			Vector2 currentMousePosition = Input.GetMousePosition();


			Vector2 delta = m_LastMousePosition - currentMousePosition;

			if (delta.X != 0.0f || delta.Y != 0.0f)
			{
				Vector3 rotation = m_Camera.Transform.Rotation;
				rotation.X += delta.Y * MouseSensitivity * Frame.TimeStep;
				rotation.Y += delta.X * MouseSensitivity * Frame.TimeStep;
				rotation.X = Mathf.Clamp(rotation.X, -Mathf.HalfPI, Mathf.HalfPI);
				m_Camera.Transform.Rotation = rotation;
			}
			//m_Rigidbody.Rotate(Vector3.Up * delta.X * MouseSensitivity * Frame.TimeStep);

			m_LastMousePosition = currentMousePosition;

			//Vector3 right = new Quaternion(m_Camera.Transform.Rotation) * Vector3.Right;

			if (Input.IsKeyDown(KeyCode.W))
			{
				m_MovementDirection.Y = -1;
			}

			if (Input.IsKeyDown(KeyCode.S))
			{
				m_MovementDirection.Y = 1;
			}
			if (Input.IsKeyDown(KeyCode.A))
			{
				m_MovementDirection.X = -1;
			}

			if (Input.IsKeyDown(KeyCode.D))
			{
				m_MovementDirection.X = 1;
			}

			Vector3 movement = right * m_MovementDirection.X + forward * m_MovementDirection.Y;
			movement.Y = 0.0f;

			if(movement.Length > 0.0f)
				movement.Normalize();
			Vector3 velocity = movement * m_Speed;
			velocity.Y = m_Rigidbody.LinearVelocity.Y;
			m_Rigidbody.LinearVelocity = velocity;

		}
	}
}
