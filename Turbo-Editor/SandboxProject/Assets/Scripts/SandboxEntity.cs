using Turbo;

namespace Sandbox
{
	public class SandboxEntity : Entity
	{
		private Rigidbody2DComponent m_Rigidbody;
		private SpriteRendererComponent m_SpriteRenderer;
		private AudioSourceComponent m_AudioSource;

		public float m_Float;
		public double m_Double;
		public bool m_Bool;
		public int m_Int;
		public uint m_UnsignedInt;
		public long m_Long;
		public ulong m_UnsignedLong;
		public sbyte m_Byte;
		public byte m_UnsignedByte;
		public Vector2 m_Vector2;
		public Vector3 m_Vector3;
		public Vector4 m_Vector4;

		// TODO: Virtual paths for Visual Studio

		protected override void OnCreate()
		{
			Log.Info($"Entity ID: {ID}!");
			Log.Warn("Hello from C#!");
			Log.Error("Hello from C#!");

			Log.Info($"Entity transform: {Transform.Translation.y}");

			m_Rigidbody = GetComponent<Rigidbody2DComponent>();
			m_Rigidbody.GravityScale = 0.0f;

			m_SpriteRenderer = GetComponent<SpriteRendererComponent>();
			m_SpriteRenderer.Color = new Vector4(1.0f, 0.0f, 1.0f, 1.0f);

			m_AudioSource = GetComponent<AudioSourceComponent>();
			m_AudioSource.PlayOnStart = true;

			this.OnCollisionBegin2D += OnCollision2D;
			this.OnTriggerBegin2D += OnTrigger2D;
		}

		protected override void OnUpdate(float ts)
		{
			if (!m_Bool)
				return;

			if (Input.IsKeyPressed(KeyCode.W))
			{
				m_Rigidbody.ApplyLinearImpulse(Vector2.Up * m_Float * ts);
			}

			if (Input.IsKeyPressed(KeyCode.S))
			{
				m_Rigidbody.ApplyLinearImpulse(Vector2.Up * -m_Float * ts);
			}

			if (Input.IsKeyPressed(KeyCode.A))
			{
				m_Rigidbody.ApplyLinearImpulse(Vector2.Right * -m_Float * ts);
			}

			if (Input.IsKeyPressed(KeyCode.D))
			{
				m_Rigidbody.ApplyLinearImpulse(Vector2.Right * m_Float * ts);
			}
		}

		private void OnCollision2D(Entity other)
		{
			Log.Info($"{other.Name}");
		}

		private void OnTrigger2D(Entity other)
		{
			Log.Info($"Trigger {other.Name}");
		}
	}
}
