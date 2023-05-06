using Turbo;

namespace GunNRun
{
	public enum PhysicsCategory : ushort
	{
		Player,
		Enemy,
		Wall
	}

	public class PlayerManager : Entity
	{
		// ---- Player Controller ----
		public float m_Speed;
		public float m_JumpPower;
		public bool m_AutoJump = false;

		// ---- Player Animation ----
		public float IdleAnimationDelay;
		public float RunningAnimationDelay;
		public float IdleShootingAnimationDelay;
		public float RunShootingAnimationDelay;
		public float InAirShootingAnimationDelay;

		private float m_OffsetX = 2.0f;
		private float m_OffsetY = -0.25f;

		// --------------------------
		internal PlayerController m_PlayerController { get; private set; } = new PlayerController();
		internal PlayerInput m_PlayerInput { get; private set; } = new PlayerInput();
		internal PlayerAnimator m_PlayerAnimator { get; private set; } = new PlayerAnimator();

		private void SpawnBullet()
		{
			//Log.Info("Pew pew!"); 
			Vector2 direction = new Vector2(Mathf.Sign(Transform.Scale.X), 0.0f);

			Vector3 translation = Transform.Translation;
			translation += new Vector3(direction.X * m_OffsetX, m_OffsetY, 0);

			Bullet bullet = Instantiate("Assets/Prefabs/Bullet.tprefab", translation).As<Bullet>();
			bullet.SetDirection(direction);
		}

		protected override void OnCreate()
		{
			m_PlayerInput.Init(this);
			m_PlayerAnimator.Init(this);
			m_PlayerController.Init(this);

			//var boxCollider = GetComponent<BoxCollider2DComponent>();
			//boxCollider.CollisionCategory = PhysicsCategory.Player;
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
