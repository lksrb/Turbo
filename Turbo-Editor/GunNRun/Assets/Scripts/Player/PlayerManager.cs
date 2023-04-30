using Turbo;

namespace GunNRun
{
	public class PlayerManager : Entity
	{
		// ---- Player Controller ----
		public float m_Speed;
		public float m_JumpPower;
		public bool m_AutoJump = false;
		public bool m_FollowsCamera = true;

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
			Log.Info("Pew pew!"); 
			Vector2 direction = new Vector2(Mathf.Sign(Transform.Scale.x), 0.0f);

			Vector3 translation = Transform.Translation;
			translation += new Vector3(direction.x * 2, -0.25f, 0);

			Bullet bullet = Instantiate("Assets/Prefabs/Bullet.tprefab", translation).As<Bullet>();
			bullet.SetDirection(direction);
		}

		protected override void OnCreate()
		{
			Log.Info("Hello entity!");

			m_PlayerInput.Init(this);
			m_PlayerAnimator.Init(this);
			m_PlayerController.Init(this);

			m_CameraEntity = FindEntityByName("Camera");
			if(m_FollowsCamera)
			{
				m_CameraEntity.Transform.Translation = new Vector3(Transform.Translation.x, Transform.Translation.y, m_CameraEntity.Transform.Translation.z);
			}

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
			if(m_FollowsCamera)
			{
				Vector3 playerTranslation = Transform.Translation;
				Entity topBoundary = FindEntityByName("TopBoundary");
				Entity bottomBoundary = FindEntityByName("BottomBoundary");

				float topY = topBoundary.Transform.Translation.y;
				float bottomY = bottomBoundary.Transform.Translation.y;

				// This is the center offset
				topY += -15f;
				bottomY += 15f;

				playerTranslation.y = Mathf.Clamp(playerTranslation.y, bottomY, topY);

				//Vector3 offset = m_CameraEntity.Transform.Translation - playerTranslation;

				Vector3 cameraTranslation = m_CameraEntity.Transform.Translation;

				float lerpSpeed = 3.0f;
				Vector3 translation = Mathf.Lerp(m_CameraEntity.Transform.Translation, playerTranslation, lerpSpeed * ts);

				float dampSpeed = 0.3f;

				Vector3 velocity = Mathf.Sign(playerTranslation - cameraTranslation);

				velocity *= 10.0f;

			/*	Vector3 translation = new Vector3(
					Mathf.SmoothDamp(cameraTranslation.x, playerTranslation.x, ref velocity.x, dampSpeed, ts, ts),
					Mathf.SmoothDamp(cameraTranslation.y, playerTranslation.y, ref velocity.y, dampSpeed, ts, ts),
					m_CameraEntity.Transform.Translation.z
					);*/

				m_CameraEntity.Transform.Translation = translation;
			}
		}
	}
}
