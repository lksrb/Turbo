using Turbo;

namespace Sandbox
{
	public class CameraController : Entity
	{
		private TransformComponent m_Transform;

		void OnStart()
		{
			Log.Info("Hello from CameraController");
			m_Transform = GetComponent<TransformComponent>();
		}

		void OnUpdate(float ts)
		{
			/*if (Input.IsKeyPressed(KeyCode.W))
			{
				m_Transform.Translation
			}

			if (Input.IsKeyPressed(KeyCode.S))
			{
			}

			if (Input.IsKeyPressed(KeyCode.A))
			{
			}

			if (Input.IsKeyPressed(KeyCode.D))
			{
			}*/
		}
	}
}
