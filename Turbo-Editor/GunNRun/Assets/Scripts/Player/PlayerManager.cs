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
		}
	}
}
