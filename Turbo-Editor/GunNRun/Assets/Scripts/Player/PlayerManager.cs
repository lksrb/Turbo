using Turbo;

namespace GunNRun
{
	public class PlayerManager : Entity
	{
		// ---- Player Controller ----
		public float m_Speed;
		public float m_JumpPower;
		public bool m_AutoJump = false;

		// ---- Player Animation ----
		public float m_AnimationIdleSpeed = 0.0f;

		// --------------------------
		internal PlayerController m_PlayerController { get; private set; } = new PlayerController();
		internal PlayerInput m_PlayerInput { get; private set; } = new PlayerInput();
		internal PlayerAnimator m_PlayerAnimator { get; private set; } = new PlayerAnimator();

		private Entity m_CameraEntity;

		~PlayerManager()
		{
			Log.Info("PlayerManager released!");
		}

		private void SpawnBullet()
		{
			// Spawn bullets
			Log.Info("Pew pew!"); // TODO: Prefabs
			Entity entity = Scene.CreateEntity("Bullet");
			entity.AttachScript("GunNRun.Bullet");
			entity.Transform.Translation = Transform.Translation;
			entity.Transform.Translation += new Vector3(Mathf.Sign(Transform.Scale.x) * 2, -0.25f, 0);
			entity.Transform.Scale = new Vector3(0.3f, 0.15f, 0.3f);

			entity.AddComponent<SpriteRendererComponent>();
			var rigidbody2D = entity.AddComponent<Rigidbody2DComponent>();

			Vector3 vel = Transform.Scale;
			vel.Normalize();
			vel.x *= 40.0f;
			vel.y = 0;
			rigidbody2D.Velocity = new Vector2(vel.x, vel.y);
		}

		protected override void OnCreate()
		{
			Log.Info("Hello entity!");

			m_PlayerInput.Init(this);
			m_PlayerAnimator.Init(this);
			m_PlayerController.Init(this);

			m_CameraEntity = FindEntityByName("Camera");
			m_CameraEntity.Transform.Translation = new Vector3(Transform.Translation.x, Transform.Translation.y, m_CameraEntity.Transform.Translation.z);

			OnCollisionBegin2D += m_PlayerController.OnCollisionBegin;
			OnCollisionEnd2D += m_PlayerController.OnCollisionEnd;
		}

		protected override void OnUpdate(float ts)
		{
			if (m_PlayerInput.IsShootKeyPressedOneTime)
			{
				SpawnBullet();
			}

			m_PlayerInput.OnUpdate(ts);
			m_PlayerController.OnUpdate(ts);
			m_PlayerAnimator.OnUpdate(ts);

			// Camera moving
			float z = m_CameraEntity.Transform.Translation.z;
			m_CameraEntity.Transform.Translation = Mathf.Lerp(m_CameraEntity.Transform.Translation, Transform.Translation, 3 * ts);
			m_CameraEntity.Transform.Translation = new Vector3(m_CameraEntity.Transform.Translation.x, m_CameraEntity.Transform.Translation.y, z);
		}
	}
}
