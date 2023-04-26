using Turbo;

namespace GunNRun
{
	public class PlayerController : Entity
	{
		public float m_Speed;
		public float m_JumpPower;
		public bool m_AutoJump = false;

		private bool m_IsGrounded = false;
		private Rigidbody2DComponent m_RigidBody2D;
		private Entity m_CameraEntity;
		private bool m_SpaceKeyReleased = true;

		~PlayerController()
		{
			Log.Info("PlayerController destroyed!");
		}

		private bool m_ShootButtonReleased = true;

		protected override void OnCreate()
		{
			Log.Info("Hello entity!");
			m_RigidBody2D = GetComponent<Rigidbody2DComponent>();
			m_CameraEntity = FindEntityByName("Camera");
		}

		protected override void OnUpdate(float ts)
		{
			m_IsGrounded = Mathf.Abs(m_RigidBody2D.Velocity.y) < ts;

			if (m_SpaceKeyReleased && m_IsGrounded && Input.IsKeyPressed(KeyCode.Space))
			{
				m_IsGrounded = false;
				m_RigidBody2D.ApplyForceToCenter(Vector2.Up * m_JumpPower);
			}

			m_SpaceKeyReleased = Input.IsKeyReleased(KeyCode.Space);

			if (m_ShootButtonReleased && Input.IsMouseButtonPressed(MouseCode.ButtonLeft))
			{
				// Spawn bullets
				Log.Info("Pew pew!"); // TODO: Prefabs
				Entity entity = Scene.CreateEntity("Bullet");
				entity.AttachScript("GunNRun.Bullet"); // FIXME: Little big problem, we do not have destroy stript because idk how gc collect work
				entity.Transform.Translation = Transform.Translation;
				entity.Transform.Scale *= 0.3f;

				entity.AddComponent<SpriteRendererComponent>();
				var rigidbody2D = entity.AddComponent<Rigidbody2DComponent>();

				//Vector3 worldPosMouse = Scene.ScreenToWorldPosition(Input.MousePosition);

				//Vector3 vel = worldPosMouse - entity.Transform.Translation;
				//vel.Normalize();

				Vector3 vel = Transform.Scale;
				vel.Normalize();
				vel.x *= 40.0f;

				rigidbody2D.Velocity = new Vector2(vel.x, vel.y);

				//Log.Info($"Spawned at: {worldPosMouse}");
			}

			m_ShootButtonReleased = Input.IsMouseButtonReleased(MouseCode.ButtonLeft);

			Vector2 velocity = new Vector2(0, m_RigidBody2D.Velocity.y);

			if (Input.IsKeyPressed(KeyCode.D))
			{
				// Rotate it to right

				Transform.Scale = Mathf.Abs(Transform.Scale);
				velocity.x = m_Speed;
			}
			if (Input.IsKeyPressed(KeyCode.A))
			{
				Vector3 rotatedScale = new Vector3(-Mathf.Abs(Transform.Scale.x), Transform.Scale.y, Transform.Scale.z);
				Transform.Scale = rotatedScale;

				velocity.x = (-1) * m_Speed;
			}

			m_RigidBody2D.Velocity = velocity;

			// Camera moving
			m_CameraEntity.Transform.Translation = Mathf.Lerp(m_CameraEntity.Transform.Translation, Transform.Translation, 3 * ts);
		}
	}
}
