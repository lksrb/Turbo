﻿using Turbo;
using System.Collections.Generic;

namespace GunNRun
{
	internal class ShooterAnimator
	{
		private static class ShooterAnimation
		{
			public static readonly int Idle = 0;
			public static readonly int Running = 1;
			public static readonly int Count = 2;
		}

		private ShooterEnemy m_ShooterEnemy;
		private SpriteAnimator m_Animator;
		private Vector2 m_SpriteSize = new Vector2(20.0f, 20.0f);

		internal void Init(ShooterEnemy shooterEnemy)
		{
			m_ShooterEnemy = shooterEnemy;
			m_Animator = new SpriteAnimator(m_ShooterEnemy.GetComponent<SpriteRendererComponent>(), ShooterAnimation.Count);

			//Idle
			{
				var frameIndicies = new List<Vector2>(3)
				{
					new Vector2(0, 1),
					new Vector2(1, 1),
					new Vector2(2, 1),
				};

				m_Animator.AddAnimation(new SpriteAnimation(ShooterAnimation.Idle, frameIndicies, m_SpriteSize, m_ShooterEnemy.IdleAnimation, true));
			}

			// Running
			{
				var frameIndicies = new List<Vector2>(4)
				{
					new Vector2(0, 0),
					new Vector2(1, 0),
					new Vector2(2, 0),
					new Vector2(3, 0)
				};

				m_Animator.AddAnimation(new SpriteAnimation(ShooterAnimation.Running, frameIndicies, m_SpriteSize, m_ShooterEnemy.RunningAnimation, true));
			}

			m_Animator.ChangeAnimation(ShooterAnimation.Idle);
		}

		internal void OnUpdate()
		{
			if (m_ShooterEnemy.Velocity.Length != 0.0f)
			{
				m_Animator.ChangeAnimation(ShooterAnimation.Running);
			}
			else
			{
				m_Animator.ChangeAnimation(ShooterAnimation.Idle);
			}

			m_Animator.OnUpdate(Frame.TimeStep);
		}
	}

	internal class ShooterGun
	{
		private Entity m_Gun;
		private ShooterEnemy m_ShooterEnemy;
		private Vector2 m_GunShooterOffset;
		private Vector2 m_ShootDirection;

		private readonly string m_BulletPrefab = "Assets/Prefabs/Bullet.tprefab";

		private Player m_Player;

		private Vector3 m_Translation;
		private Vector3 m_Scale;
		private Vector3 m_Rotation;

		private AudioSourceComponent m_RifleShootAudio;
		private Timer m_DeltaShootTimer = new Timer(0.05f);
		private Timer m_TimeBetweenBurstShot = new Timer(1.7f, false);
		private bool m_CanShoot = false;
		private bool m_StartBurst = true;

		internal void Init(ShooterEnemy enemy, Player player)
		{
			m_ShooterEnemy = enemy;
			m_Player = player;
			m_Gun = m_ShooterEnemy.GetChildren()[0];

			m_GunShooterOffset = new Vector2(-2.74f + 2.64f, -0.45f + 0.21f);

			m_RifleShootAudio = m_Gun.GetComponent<AudioSourceComponent>();
		}

		internal void OnUpdate()
		{
			// Reducing the amount of dll calls
			m_Translation = m_Gun.Transform.Translation;
			m_Scale = m_Gun.Transform.Scale;

			OnMoveToPlayer();
			OnRotateToPlayer();

			m_Translation.Z = 0.40f;

			if (m_CanShoot && m_StartBurst && m_DeltaShootTimer)
			{
				ShootBullet();
			}

			if (m_CanShoot && m_TimeBetweenBurstShot)
			{
				m_TimeBetweenBurstShot.Reset();
				m_StartBurst = !m_StartBurst;
			}


			if (m_CanShoot && m_StartBurst)
			{
				m_RifleShootAudio.Play();
			}
			else
			{
				m_RifleShootAudio.Stop();
			}

			// Set modified variables
			m_Gun.Transform.Translation = m_Translation;
			m_Gun.Transform.Rotation = m_Rotation;
			m_Gun.Transform.Scale = m_Scale;
		}

		internal void StartShooting()
		{
			if(!m_CanShoot)
			{
				m_StartBurst = true;
			}

			m_CanShoot = true;
		}

		internal void StopShooting()
		{
			if (m_CanShoot)
			{
				m_StartBurst = false;
				m_DeltaShootTimer.Reset();
				m_TimeBetweenBurstShot.Reset();
			}

			m_CanShoot = false;
		}


		private void ShootBullet()
		{
			var bulletTranslation = m_Translation + m_ShootDirection * 0.45f;
			bulletTranslation.Y += 0.1f;
			bulletTranslation.Z = 0.5f;
			Bullet bullet = m_ShooterEnemy.InstantiateChild(m_BulletPrefab, bulletTranslation).As<Bullet>();
			bullet.Transform.Scale *= 0.5f;
			bullet.Init(m_ShooterEnemy, m_ShootDirection);
		}

		private void OnMoveToPlayer()
		{
			// Gun movement & rotation
			m_Translation.XY = m_ShooterEnemy.Translation.XY + m_GunShooterOffset;
		}

		private void OnRotateToPlayer()
		{
			m_ShootDirection = m_Player.Transform.Translation - m_Translation;
			m_ShootDirection.Normalize();

			float angle = Mathf.Atan(m_ShootDirection.Y / m_ShootDirection.X); // [-90,90]

			m_Scale.Y = m_ShootDirection.X >= 0.0f ? Mathf.Abs(m_Scale.Y) : -Mathf.Abs(m_Scale.Y);
			m_Rotation.Z = m_ShootDirection.X >= 0.0f ? angle : angle + Mathf.PI;
		}
	}

	public class ShooterEnemy : Entity
	{
		public readonly float Speed;
		public readonly float IdleAnimation;
		public readonly float RunningAnimation;

		public float MinDistanceFromPlayer = 5.0f;

		internal Vector2 Velocity { get; private set; } = Vector2.Zero;
		internal Vector3 Translation { get; private set; }
		internal Vector3 Scale { get; private set; }

		// Movement
		private Player m_Player;
		private Rigidbody2DComponent m_Rigidbody2D;
		private bool m_IsMoving = true;
		private bool m_CanMove = true;
		private Timer m_WaitAfterPlayerDistance = new Timer(3.0f);
		private SingleTickTimer m_DeathTimer = new SingleTickTimer(0.1f);

		private bool m_Destroy;

		// Animation
		private ShooterAnimator m_Animator = new ShooterAnimator();
		private ShooterGun m_Gun = new ShooterGun();

		// Particles
		private ParticleSystem m_DeathParticles;
		private LevelManager m_LevelManager;

		protected override void OnCreate()
		{
			Translation = Transform.Translation;
			Scale = Transform.Scale;
			m_Player = FindEntityByName("Player").As<Player>();
			m_Rigidbody2D = GetComponent<Rigidbody2DComponent>();
			m_LevelManager = FindEntityByName("GameManager").As<GameManager>().GetLevelManager();

			m_DeathParticles = ParticleSystem.Setup()
					.SetName("EnemyDeathParticles")
					.AddColor(Color.Red / 1.6f)
					.AddColor(Color.Red / 1.4f)
					.AddColor(Color.Red / 1.2f)
					.AddColor(Color.Red / 1.2f)
					.AddColor(Color.Red / 1.2f)
					.AddColor(Color.Red / 1.2f)
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

			m_Animator.Init(this);
			m_Gun.Init(this, m_Player);

			OnCollisionBegin2D += OnTakeHit;
		}

		protected override void OnUpdate()
		{
			Translation = Transform.Translation;
			Scale = Transform.Scale;

			OnMovement();
			m_Gun.OnUpdate();
			OnEnemyLogic();
			m_Animator.OnUpdate();

			if (m_Destroy)
			{
				if (m_DeathTimer)
				{
					// Death sound effect
					Instantiate("Assets/Prefabs/EnemyDeathSoundEffect.tprefab", Translation);

					m_DeathParticles.Start(Transform.Translation);
					m_Destroy = false;
					m_LevelManager.OnEnemyDestroyed(this);
					Scene.DestroyEntity(this);
				}
			}
		}

		bool SquareCollision(Vector3 enemyDistance, float maxDistance) => Mathf.Abs(enemyDistance.X) < maxDistance && Mathf.Abs(enemyDistance.Y) < maxDistance;
		bool CircleCollision(Vector3 enemyDistance, float maxDistance) => enemyDistance.Length < maxDistance;

		private void OnMovement()
		{
			Vector3 distance = m_Player.Transform.Translation - Translation;

			bool collided = false;

			foreach (Entity enemy in m_LevelManager.CurrentEnemies)
			{
				if (enemy != this)
				{
					Vector3 enemyDistance = enemy.Transform.Translation - Translation;

					if (CircleCollision(enemyDistance, 2.0f))
					{
						Log.Info("Shooter enemies collided!");
						collided = true;
						break;
					}
				}
			}

			if (!SquareCollision(distance, MinDistanceFromPlayer) && !collided && m_CanMove)
			{
				Velocity = Mathf.Normalize(distance) * Speed;
			}
			else
			{
				Velocity = Vector2.Zero;
				m_CanMove = m_WaitAfterPlayerDistance;
			}

			Vector3 scale = Scale;
			scale.X = distance.X > 0.0f ? Mathf.Abs(scale.X) : -Mathf.Abs(scale.X);
			Scale = scale;

			m_IsMoving = Velocity.Length != 0.0f;

			Transform.Scale = Scale;
			m_Rigidbody2D.Velocity = Velocity;
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
