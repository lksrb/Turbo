using Turbo;

namespace Sandbox
{
	public class ExampleEntity : Entity
    {
		float m_Speed = 20.0f;
		Entity m_Camera;

		public float DistanceFromPlayer = 8.0f;
		public float MouseSensitivity = 1.0f;

		private Vector2 m_LastMousePosition;

		protected override void OnCreate()
        {
			m_Camera = FindEntityByName("Camera");

			Input.SetCursorMode(CursorMode.Locked);
			//DistanceFromPlayer = Mathf.Length(Transform.Translation - m_Camera.Transform.Translation);

			Log.Info("Hello entity!");
		}

        protected override void OnUpdate()
        {
			{
				Vector3 translation = Transform.Translation;

				if (Input.IsKeyDown(KeyCode.W))
				{
					translation.Z -= Frame.TimeStep * m_Speed;
				}

				if (Input.IsKeyDown(KeyCode.S))
				{
					translation.Z += Frame.TimeStep * m_Speed;
				}
				if (Input.IsKeyDown(KeyCode.A))
				{
					translation.X -= Frame.TimeStep * m_Speed;
				}

				if (Input.IsKeyDown(KeyCode.D))
				{
					translation.X += Frame.TimeStep * m_Speed;
				}

				Transform.Translation += forward * Frame.TimeStep;
			}

			Vector3 forward = new Quaternion(m_Camera.Transform.Rotation) * Vector3.Forward;
			Log.Info(forward);

			// Camera movement
			Vector2 currentMousePosition = Input.GetMousePosition();
			if (true)
			{
				Vector2 delta = m_LastMousePosition - currentMousePosition;
				Vector3 rotation = m_Camera.Transform.Rotation;
				rotation.Y += (delta.X * Frame.TimeStep) % Mathf.TwoPI;
				m_Camera.Transform.Rotation = rotation;
			}


			m_LastMousePosition = currentMousePosition;
		}
    }
}
