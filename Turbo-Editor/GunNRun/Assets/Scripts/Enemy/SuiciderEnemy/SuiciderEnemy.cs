using Turbo;

namespace GunNRun
{
	public class SuiciderEnemy : Entity
	{
		public readonly float Speed;
		public readonly float RunningAnimation;

		private Rigidbody2DComponent m_Rigidbody2D;
		private SpriteRendererComponent m_SpriteRenderer;
		private Player m_Player;
		private SuiciderAnimator m_Animator;
		private System.Action<Entity> m_OnDestroyCallback;

		private Vector3 m_Translation = Vector3.Zero;
		private Vector3 m_Scale = Vector3.Zero;
		private Vector2 m_Velocity = Vector3.Zero;

		private SingleTickTimer m_DeathTimer = new SingleTickTimer(0.1f);
		private GameManager m_GameManager;

		// Maximum distance from player
		private float m_MaxDistanceLength;
		private readonly float m_HitRadius = 1.3f;
		private bool m_Destroy = false;

		// Particles
		private ParticleSystem m_DeathParticles;

		protected override void OnCreate()
		{
			m_Rigidbody2D = GetComponent<Rigidbody2DComponent>();
			m_Player = FindEntityByName("Player").As<Player>();
			m_SpriteRenderer = GetComponent<SpriteRendererComponent>();
			m_GameManager = FindEntityByName("GameManager").As<GameManager>();

			m_DeathParticles = ParticleSystem.Setup()
					.SetName("EnemyExplosion")
					.AddColor(Color.Red / 1.6f)
					.AddColor(Color.Red / 1.2f, 4)
					.AddColor(Color.Red / 1.4f)
					.AddColor(Color.Orange, 5)
					.AddColor(Color.Orange / 1.2f, 5)
					.AddColor(Color.Orange / 1.3f, 5)
					.SetDurationRange(0.25f, 0.50f)
					.SetVelocityRange(5.0f, 10.0f)
					.SetRotationVelocityRange(-10.0f, 10.0f)
					.SetScale(0.15f, 0.15f)
					.Build(40);

			// Setup filter categories
			CollisionFilter filter = new CollisionFilter();
			filter.CollisionCategory = (ushort)EntityCategory.Enemy;
			filter.CollisionMask = (ushort)EntityCategory.Enemy | (ushort)EntityCategory.Bullet | (ushort)EntityCategory.Wall;

			GetComponent<BoxCollider2DComponent>().Filter = filter;

			m_Animator = new SuiciderAnimator(this);

			m_MaxDistanceLength = (m_Player.Transform.Translation - Transform.Translation).Length;

			OnCollisionBegin2D += OnBulletHit;
		}

		internal void SetOnDestroyCallback(System.Action<Entity> onDestroyCallback)
		{
			m_OnDestroyCallback = onDestroyCallback;
		}

		protected override void OnUpdate()
		{
			if(m_GameManager.CurrentGameState == GameState.GameOver)
			{
				m_Destroy = true;
			}

			OnMovement();
			m_Animator.OnUpdate();

			if (m_Destroy)
			{
				if (m_DeathTimer)
				{
					SoundEffect.Play(Effect.EnemyDeath, m_Translation);

					Vector3 distance = m_Player.Transform.Translation - m_Translation;

					if(distance.Length < m_HitRadius)
					{
						m_Player.TakeDamage(this);
					}

					m_DeathParticles.Start(m_Translation);
					m_OnDestroyCallback?.Invoke(this);
					Scene.DestroyEntity(this);
				}
			}
		}

		private void OnMovement()
		{
			m_Translation = Transform.Translation;
			m_Scale = Transform.Scale;

			Vector3 distance = m_Player.Transform.Translation - m_Translation;

			m_Velocity = Mathf.Normalize(distance) * Speed;

			// Change color according to player distance
			Color color = m_SpriteRenderer.SpriteColor;
			color.G = Mathf.Normalize(distance.Length, 0.0f, m_MaxDistanceLength) * 0.7f;
			color.B = Mathf.Normalize(distance.Length, 0.0f, m_MaxDistanceLength);
			m_SpriteRenderer.SpriteColor = color;

			if(distance.Length < m_HitRadius)
			{
				m_Destroy = true;
			}

			// Rotate towards player
			Vector3 scale = m_Scale;
			scale.X = distance.X > 0.0f ? Mathf.Abs(scale.X) : -Mathf.Abs(scale.X);
			m_Scale = scale;

			Transform.Scale = m_Scale;
			m_Rigidbody2D.Velocity = m_Velocity;
		}

		private void OnBulletHit(Entity other)
		{
			if (other.Name == "Bullet")
			{
				Bullet bullet = other.As<Bullet>();

				if (bullet.ShooterEntity.Name == "Shooter" || bullet.ShooterEntity.Name == "Sniper")
					return;

				m_Destroy = true;
			}
		}
	}
}
