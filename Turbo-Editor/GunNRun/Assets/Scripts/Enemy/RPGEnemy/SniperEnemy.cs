using Turbo;

namespace GunNRun
{
	public class SniperEnemy : Entity
	{
		public readonly float Speed;
		public readonly float IdleAnimation;

		internal Vector2 Velocity => m_Velocity;
		internal Vector3 Translation => m_Translation;

		private SniperAnimator m_Animator;
		private SniperGun m_SniperGun;
		private Player m_Player;
		private Vector2 m_Velocity = Vector2.Zero;
		private Vector3 m_Translation = Vector3.Zero;
		private Vector3 m_Scale = Vector3.Zero;
		private Rigidbody2DComponent m_Rigidbody2D;

		private bool m_Destroy = false;
		private SingleTickTimer m_DeathTimer = new SingleTickTimer(0.1f);
		private System.Action<Entity> m_OnDestroyCallback;

		// Particles
		private ParticleSystem m_DeathParticles;
		private GameManager m_GameManager;

		private Timer m_ShootTimer = new Timer(2.5f, false);
		private Timer m_ShootCooldown = new Timer(2.5f);
		private bool m_ShootOnce = true;

		private LineRendererComponent m_LineRenderer;


		protected override void OnCreate()
		{
			m_Player = FindEntityByName("Player").As<Player>();
			m_Rigidbody2D = GetComponent<Rigidbody2DComponent>();
			m_GameManager = FindEntityByName("GameManager").As<GameManager>();

			m_LineRenderer = GetComponent<LineRendererComponent>();
			m_LineRenderer.LineColor = Color.Red;

			// Setup filter categories
			CollisionFilter filter = new CollisionFilter();
			filter.CollisionCategory = (ushort)EntityCategory.Enemy;
			filter.CollisionMask = (ushort)EntityCategory.Enemy | (ushort)EntityCategory.Bullet | (ushort)EntityCategory.Wall;
			GetComponent<BoxCollider2DComponent>().Filter = filter;

			m_DeathParticles = ParticleSystem.Setup()
					.SetName("EnemyDeathParticles")
					.AddColor(Color.Red / 1.6f)
					.AddColor(Color.Red / 1.4f)
					.AddColor(Color.Red / 1.2f, 4)
					.AddColor(Color.Red)
					.AddColor(Color.Black / 1.4f, 2)
					.AddColor(Color.Black / 1.2f, 2)
					.SetDurationRange(0.25f, 0.50f)
					.SetVelocityRange(5.0f, 10.0f)
					.SetRotationVelocityRange(-10.0f, 10.0f)
					.SetScale(0.15f, 0.15f)
					.Build(25);

			m_Animator = new SniperAnimator(this);
			m_SniperGun = new SniperGun(this);

			OnCollisionBegin2D += OnBulletHit;
		}

		protected override void OnUpdate()
		{
			if (m_GameManager.CurrentGameState == GameState.GameOver)
			{
				m_Destroy = true;
			}

			m_Translation = Transform.Translation;
			m_Scale = Transform.Scale;

			m_Animator.OnUpdate();
			m_SniperGun.OnUpdate();

			Vector3 distance = m_Player.Transform.Translation - m_Translation;

			// Rotate towards player
			Vector3 scale = m_Scale;
			scale.X = distance.X > 0.0f ? Mathf.Abs(scale.X) : -Mathf.Abs(scale.X);
			m_Scale = scale;

			if (m_ShootTimer)
			{
				m_LineRenderer.LineColor = Color.Clear;

				if (m_ShootOnce)
				{
					m_ShootOnce = false;
					SoundEffect.Play(Effect.Sniper, m_Translation);
					m_SniperGun.Shoot();
				}

				if (m_ShootCooldown)
				{
					m_ShootTimer.Reset();
				}
			}
			else
			{
				m_ShootOnce = true;
				m_LineRenderer.LineColor = Color.Red;
			}

			// Sniper line
			{
				var pos0 = m_SniperGun.Translation;
				pos0.Z = 0.30f;

				var pos1 = m_Player.Transform.Translation;
				pos1.Z = 0.30f;

				m_LineRenderer.Position0 = pos0;
				m_LineRenderer.Position1 = pos1;
			}

			if (m_Destroy)
			{
				if (m_DeathTimer)
				{
					SoundEffect.Play(Effect.EnemyDeath, m_Translation);

					m_Player.AddScore(70);
					m_OnDestroyCallback?.Invoke(this);
					m_DeathParticles.Start(Transform.Translation);
					m_Destroy = false;
					Scene.DestroyEntity(this);
				}
			}

			Transform.Scale = m_Scale;
			m_Rigidbody2D.Velocity = m_Velocity;
		}

		internal void SetOnDestroyCallback(System.Action<Entity> onDestroyCallback)
		{
			m_OnDestroyCallback = onDestroyCallback;
		}

		private void OnBulletHit(Entity other)
		{
			if (other.Name == "Bullet")
			{
				Bullet bullet = other.As<Bullet>();

				if (bullet.ShooterEntity.Name == "Sniper")
					return;

				m_Destroy = true;
			}
		}

	}
}
