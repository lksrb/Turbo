using Turbo;

namespace Sandbox
{
	public class SandboxEntity : Entity
	{
		private Rigidbody2DComponent m_Rigidbody;
		private SpriteRendererComponent m_SpriteRenderer;

		public float m_Speed = 10.0f;

		void OnStart()
		{
			Log.Info("Hello from C#!");
			Log.Warn("Hello from C#!");

			Log.Info($"Entity ID: {ID}!");

			Log.Info($"Entity transform: {transform.Translation.y}");

			m_Rigidbody = GetComponent<Rigidbody2DComponent>();
			m_Rigidbody.Gravity = false;

			m_SpriteRenderer = GetComponent<SpriteRendererComponent>();

			//m_SpriteRenderer.Color = new Vector4(0, 1, 1, 1);
		}

		void OnUpdate(float ts)
		{
			if (Input.IsKeyPressed(KeyCode.W))
			{
				m_Rigidbody.ApplyLinearImpulse(Vector2.Up * m_Speed * ts);
			}

			if (Input.IsKeyPressed(KeyCode.S))
			{
				m_Rigidbody.ApplyLinearImpulse(Vector2.Up * -m_Speed * ts);
			}

			if (Input.IsKeyPressed(KeyCode.A))
			{
				m_Rigidbody.ApplyLinearImpulse(Vector2.Right * -m_Speed * ts);
			}

			if (Input.IsKeyPressed(KeyCode.D))
			{
				m_Rigidbody.ApplyLinearImpulse(Vector2.Right * m_Speed * ts);
			}
		}
	}
}