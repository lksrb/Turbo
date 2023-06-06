using Turbo;

namespace GunNRun
{
	public class ShooterEnemy : Entity
	{
		public readonly float Speed;
		public readonly float IdleAnimation;
		public readonly float RunningAnimation;

		internal Vector2 Velocity => m_Velocity;
		internal Vector3 Translation => m_Translation;

		// Movement
		private Player m_Player;
		private Rigidbody2DComponent m_Rigidbody2D;
		private bool m_IsMoving = true;
		private bool m_CanMove = true;
		private Timer m_WaitAfterPlayerDistance = new Timer(3.0f);
		private SingleTickTimer m_DeathTimer = new SingleTickTimer(0.1f);
		private Vector2 m_Velocity = Vector2.Zero;
		private Vector3 m_Translation = Vector3.Zero;
		private Vector3 m_Scale = Vector3.Zero;
		private readonly float m_MinDistanceFromPlayer = 5.0f;

		private bool m_Destroy = false;
		private System.Action<Entity> m_OnDestroyCallback;

		// Animation
		private ShooterAnimator m_Animator;
		private ShooterGun m_Gun;

		// Particles
		private ParticleSystem m_DeathParticles;

		private GameManager m_GameManager;

		protected override void OnCreate()
		{
			m_Translation = Transform.Translation;
			m_Scale = Transform.Scale;
			m_Player = FindEntityByName("Player").As<Player>();
			m_Rigidbody2D = GetComponent<Rigidbody2DComponent>();
			m_GameManager = FindEntityByName("GameManager").As<GameManager>();

			m_DeathParticles = ParticleSystem.Setup()
					.SetName("EnemyDeathParticles")
					.AddColor(Color.Red / 1.6f)
					.AddColor(Color.Red / 1.4f)
					.AddColor(Color.Red / 1.2f, 4)
					.AddColor(Color.Red / 1.0f)
					.SetDurationRange(0.25f, 0.50f)
					.SetVelocityRange(5.0f, 10.0f)
					.SetRotationVelocityRange(-10.0f, 10.0f)
					.SetScale(0.15f, 0.15f)
					.Build(25);

			// Setup filter categories
			CollisionFilter filter = new CollisionFilter();
			filter.CollisionCategory = (ushort)EntityCategory.Enemy;
			filter.CollisionMask = (ushort)EntityCategory.Enemy | (ushort)EntityCategory.Bullet | (ushort)EntityCategory.Wall;

			GetComponent<BoxCollider2DComponent>().Filter = filter;

			m_Animator = new ShooterAnimator(this);
			m_Gun = new ShooterGun(this, m_Player);

			OnCollisionBegin2D += OnTakeHit;
		}

		protected override void OnUpdate()
		{
			if (m_GameManager.CurrentGameState == GameState.GameOver)
			{
				m_Destroy = true;
			}

			OnMovement();
			m_Gun.OnUpdate();
			OnEnemyLogic();
			m_Animator.OnUpdate();

			if (m_Destroy)
			{
				if (m_DeathTimer)
				{
					SoundEffect.Play(Effect.EnemyDeath, m_Translation);

					m_Player.AddScore(45);
					m_OnDestroyCallback?.Invoke(this);
					m_DeathParticles.Start(Transform.Translation);
					m_Destroy = false;
					Scene.DestroyEntity(this);
				}
			}
		}

		bool SquareCollision(Vector3 enemyDistance, float maxDistance) => Mathf.Abs(enemyDistance.X) < maxDistance && Mathf.Abs(enemyDistance.Y) < maxDistance;
		bool CircleCollision(Vector3 enemyDistance, float maxDistance) => enemyDistance.Length < maxDistance;

		internal void SetOnDestroyCallback(System.Action<Entity> onDestroyCallback)
		{
			m_OnDestroyCallback = onDestroyCallback;
		}

		private void OnMovement()
		{
			m_Translation = Transform.Translation;
			m_Scale = Transform.Scale;

			Vector3 distance = m_Player.Transform.Translation - m_Translation;

			bool collided = false;

			// Enemy collisions

			if (!SquareCollision(distance, m_MinDistanceFromPlayer) && !collided && m_CanMove)
			{
				m_Velocity = Mathf.Normalize(distance) * Speed;
			}
			else
			{
				m_Velocity = Vector2.Zero;
				m_CanMove = m_WaitAfterPlayerDistance;
			}

			// Rotate towards player
			Vector3 scale = m_Scale;
			scale.X = distance.X > 0.0f ? Mathf.Abs(scale.X) : -Mathf.Abs(scale.X);
			m_Scale = scale;

			m_IsMoving = m_Velocity.Length != 0.0f;

			Transform.Scale = m_Scale;
			m_Rigidbody2D.Velocity = m_Velocity;
		}

		private void OnEnemyLogic()
		{
			if (!m_IsMoving)
			{
				m_Gun.StartShooting();
			}
			else
			{
				m_Gun.StopShooting();
			}
		}

		private void OnTakeHit(Entity other)
		{
			if (other.Name == "Bullet")
			{
				Bullet bullet = other.As<Bullet>();

				if (bullet.ShooterEntity.Name == "Shooter")
					return;

				m_Destroy = true;
			}
		}
	}
}
