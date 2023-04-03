using Turbo;

namespace Sandbox
{
	public class CameraController : Entity
	{
		private Vector3 m_LastPosition = new Vector3(0.0f);
		private TransformComponent m_Transform;

		void OnStart()
		{
			Log.Info("Hello from CameraController");
			m_Transform = GetComponent<TransformComponent>();
		}

		void OnUpdate(float ts)
		{
			Vector3 velocity = m_LastPosition - m_Transform.Translation;

			if(Input.IsKeyPressed(KeyCode.Left))
			{
				//m_Transform.Translation = new Vector3(m_Transform.Translation);
			}

			Log.Info(velocity.ToString());

			m_LastPosition = transform.Translation;
		}
	}
}
