using Turbo;

namespace GunNRun
{
	public class Player : Entity
	{
		// Set in editor
		public readonly float Speed;
		public readonly float IdleAnimation;
		public readonly float RunningAnimation;

		// Player components
		internal PlayerInput PlayerInput { get; private set; }
		internal PlayerController Controller { get; private set; }
		internal PlayerAnimator Animator { get; private set; }
		internal PlayerGun Gun { get; private set; }

		// Stats
		internal int AmmoCount { get; private set; } = 60;
		internal int HP { get; private set; } = 100;
		internal int ScoreCount { get; private set; } = 0;
		internal Vector2 Velocity => Controller.Velocity;

		private GameManager m_GameManager;
		private ParticleSystem m_DeathParticles;

		protected override void OnCreate()
		{
			m_GameManager = FindEntityByName("GameManager").As<GameManager>();

			// Collision masking
			CollisionFilter filter = new CollisionFilter();
			filter.CollisionCategory = (ushort)EntityCategory.Player;
			filter.CollisionMask = (ushort)EntityCategory.Everything;
			GetComponent<BoxCollider2DComponent>().Filter = filter;

			// Creating particle for later
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

			// Initializing player components
			PlayerInput = new PlayerInput();
			Controller = new PlayerController(this);
			Animator = new PlayerAnimator(this);
			Gun = new PlayerGun(this);

			OnCollisionBegin2D += OnBulletHit;

			Log.Warn("Player initialized!");
		}

		protected override void OnUpdate()
		{
			if (m_GameManager.CurrentGameState == GameState.GameOver)
			{
				m_DeathParticles.Start(Transform.Translation);
				GetComponent<SpriteRendererComponent>().SpriteColor = Color.Clear;
				Gun.Hide();
				return;
			}

			PlayerInput.OnUpdate();
			Controller.OnUpdate();
			Animator.OnUpdate();
			Gun.OnUpdate();

			if (AmmoCount > 0 && Gun.IsReady && PlayerInput.IsShootMouseButtonPressed)
			{
				AmmoCount--;

				Gun.Shoot();
			}
		}

		internal void PickItem(Entity item)
		{
			if (item.Name == "HpDrop")
			{
				HP = Mathf.Min(HP + 15, 100);
				SoundEffect.Play(Effect.PickHP, Transform.Translation);
			}
			else if (item.Name == "AmmoDrop")
			{
				AmmoCount = Mathf.Min(AmmoCount + 5, 60);
				SoundEffect.Play(Effect.PickAmmo, Transform.Translation);
			}
		}

		internal void AddScore(Entity enemy)
		{
			int score = 0;

			switch (enemy.Name)
			{
				case "Suicider": score = 25; break;
				case "Shooter": score = 70; break;
				case "Sniper": score = 45; break;
			}

			AddScore(score);
		}

		internal void TakeDamage(Entity enemy)
		{
			int damage = 0;

			switch (enemy.Name)
			{
				case "Suicider": damage = 15; break;
				case "Shooter": damage = 1; break;
				case "Sniper": damage = 20; break;
			}

			TakeDamage(damage);
		}

		private void OnBulletHit(Entity other)
		{
			if (other.Name != "Bullet")
				return;

			Bullet bullet = other.As<Bullet>();

			if (bullet.ShooterEntity.Name == "Player")
				return;

			TakeDamage(bullet.ShooterEntity);
		}

		private void TakeDamage(int damage)
		{
			if (damage <= 0 || HP <= 0)
				return;

			HP = Mathf.Max(HP - damage, 0);
		}

		private void AddScore(int score)
		{
			if (score <= 0)
				return;

			ScoreCount += score;
		}
	}
}
