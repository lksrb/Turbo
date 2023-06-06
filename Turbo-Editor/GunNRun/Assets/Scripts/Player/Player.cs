using Turbo;

namespace GunNRun
{
	public class Player : Entity
	{
		public readonly float Speed;
		public readonly float IdleAnimation;
		public readonly float RunningAnimation;

		internal readonly PlayerInput PlayerInput = new PlayerInput();
		internal readonly PlayerController Controller = new PlayerController();
		internal readonly PlayerAnimator Animator = new PlayerAnimator();
		internal readonly PlayerGun Gun = new PlayerGun();

		internal Vector2 Velocity => Controller.Velocity;
		internal SpriteRendererComponent SpriteRenderer => GetComponent<SpriteRendererComponent>();

		internal int HP => m_HP;
		internal int AmmoCount { get => m_AmmoCount; set { m_AmmoCount = value; } }
		internal int ScoreCount => m_ScoreCount;

		private GameManager m_GameManager;
		private ParticleSystem m_DeathParticles;
		private int m_HP = 100, m_AmmoCount = 60, m_ScoreCount = 0;
		private CollisionFilter m_Filter = new CollisionFilter();

		protected override void OnCreate()
		{
			m_GameManager = FindEntityByName("GameManager").As<GameManager>();

			Controller.Init(this);
			Animator.Init(this);
			Gun.Init(this);

			m_DeathParticles = ParticleSystem.Setup()
				.SetName("PlayerDeathParticles")
				.AddColor(Color.Red / 1.6f)
				.AddColor(Color.Red / 1.4f)
				.AddColor(Color.Red / 1.2f, 4)
				.AddColor(Color.Red / 1.0f)
				.AddColor(Color.Blue / 1.0f)
				.AddColor(Color.Yellow / 1.0f)
				.AddColor(Color.Green / 1.0f)
				.AddColor(Random.Float4(), 5)
				.SetDurationRange(0.25f, 0.50f)
				.SetVelocityRange(5.0f, 10.0f)
				.SetRotationVelocityRange(-10.0f, 0.0f)
				.SetScale(0.15f, 0.15f)
				.Build(35);

			m_Filter.CollisionCategory = (ushort)EntityCategory.Player;
			m_Filter.CollisionMask = (ushort)EntityCategory.Everything;
			GetComponent<BoxCollider2DComponent>().Filter = m_Filter;

			OnCollisionBegin2D += OnTakeHit;
		}

		protected override void OnUpdate()
		{
			if (m_GameManager.CurrentGameState == GameState.GameOver)
			{
				GetComponent<SpriteRendererComponent>().SpriteColor = Color.Clear;
				Gun.Hide();
				return;
			}

			PlayerInput.OnUpdate();
			Controller.OnUpdate();
			Animator.OnUpdate();
			Gun.OnUpdate();

			if (PlayerInput.IsShootMouseButtonPressed)
			{
				Gun.Shoot();
			}
		}

		private void OnTakeHit(Entity other)
		{
			if (other.Name == "Bullet")
			{
				Bullet bullet = other.As<Bullet>();

				if (bullet.ShooterEntity.Name == "Player")
					return;
				else if(bullet.ShooterEntity.Name == "Shooter")
					TakeDamage(1);
				else if (bullet.ShooterEntity.Name == "Sniper")
					TakeDamage(20);
			}
		}

		internal void PickItem(Entity item)
		{
			if (item.Name == "HpDrop")
			{
				m_HP = Mathf.Min(m_HP + 15, 100);
				SoundEffect.Play(Effect.PickHP, Transform.Translation);
			}
			else if (item.Name == "AmmoDrop")
			{
				m_AmmoCount = Mathf.Min(m_AmmoCount + 5, 60);
				SoundEffect.Play(Effect.PickAmmo, Transform.Translation);
			}
		}

		internal void TakeDamage(int damage)
		{
			if (damage == 0 || m_HP <= 0)
				return;

			m_HP = Mathf.Max(m_HP - damage, 0);

			if (m_HP == 0)
			{
				m_DeathParticles.Start(Transform.Translation);
			}
		}

		internal void AddScore(int score)
		{
			if (score <= 0)
				return;

			m_ScoreCount += score;
		}
	}
}
