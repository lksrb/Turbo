using Turbo;

namespace Sandbox
{
	public class ExampleEntity : Entity
	{
		float m_Speed = 50.0f;
		Entity m_Camera;

		public float MouseSensitivity;
		private bool m_NoClip = false;
		private Vector2 m_LastMousePosition;
		private bool m_MouseRotation = true;

		protected override void OnCreate()
		{
			m_Camera = FindEntityByName("Camera");

			Input.SetCursorMode(CursorMode.Locked);
			//DistanceFromPlayer = Mathf.Length(Transform.Translation - m_Camera.Transform.Translation);

			Log.Info("Hello entity!");
		}

		protected override void OnUpdate()
		{
			m_Speed = Input.IsKeyUp(KeyCode.LeftShift) ? 50.0f : 100.0f;

			Vector3 forward = new Quaternion(Transform.Rotation) * Vector3.Forward;
			Vector3 right = new Quaternion(Transform.Rotation) * Vector3.Right;
			{
				Vector3 translation = Transform.Translation;

				if (Input.IsKeyDown(KeyCode.W))
				{
					//translation.Z -= Frame.TimeStep * m_Speed;
					translation -= forward * Frame.TimeStep * m_Speed;
				}

				if (Input.IsKeyDown(KeyCode.S))
				{
					translation += forward * Frame.TimeStep * m_Speed;
				}
				if (Input.IsKeyDown(KeyCode.A))
				{
					translation -= right * Frame.TimeStep * m_Speed;
				}

				if (Input.IsKeyDown(KeyCode.D))
				{
					translation += right * Frame.TimeStep * m_Speed;
				}

				Transform.Translation = new Vector3(translation.X, m_NoClip == false ? Transform.Translation.Y : translation.Y, translation.Z);
			}

			if (Input.IsKeyDown(KeyCode.Escape))
			{
				m_MouseRotation = false;
			}

			if (Input.IsKeyDown(KeyCode.P))
			{
				m_MouseRotation = true;
				Input.SetCursorMode(CursorMode.Locked);
			}

			OnUpdateInput();

			if (IsShootMouseButtonPressed)
			{
				m_NoClip = !m_NoClip;
			}

			// Rotation
			Vector2 currentMousePosition = Input.GetMousePosition();
			if (m_MouseRotation)
			{
				Vector2 delta = m_LastMousePosition - currentMousePosition;
				if (delta.X != 0.0f || delta.Y != 0.0f)
				{
					Vector3 rotation = Transform.Rotation;
					rotation.X += delta.Y * MouseSensitivity * Frame.TimeStep;
					rotation.Y += delta.X * MouseSensitivity * Frame.TimeStep;
					rotation.X = Mathf.Clamp(rotation.X, -Mathf.HalfPI, Mathf.HalfPI);
					Transform.Rotation = rotation;
				}
			}

			m_LastMousePosition = currentMousePosition;
		}

		// Shooting
		private bool m_IsShootKeyReleased = true;
		private bool m_WasShootMouseButtonPressed = false;
		private bool m_IsShootMouseButtonPressed = false;
		internal bool IsShootMouseButtonPressed => !m_WasShootMouseButtonPressed && m_IsShootMouseButtonPressed;

		private void OnUpdateInput()
		{
			// Shooting
			m_WasShootMouseButtonPressed = !m_IsShootKeyReleased;
			m_IsShootMouseButtonPressed = Input.IsKeyDown(KeyCode.Q);
			m_IsShootKeyReleased = Input.IsKeyUp(KeyCode.Q);
		}
	}
}
