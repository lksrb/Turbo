using System;
using System.Runtime.InteropServices;
using Turbo;

namespace GunNRun
{
	public class Enemy : Entity
	{
		private EnemyAnimator m_Animator = new EnemyAnimator();

		// Shooting
		private readonly float m_ShootingCooldown = 0.3f;
		private float m_ShootingTimer = 0.2f;
		private readonly string m_BulletPrefab = "Assets/Prefabs/Bullet.tprefab";
		private float m_OffsetX = 1.0f;
		private float m_OffsetY = -0.25f;

		internal bool IsShooting { get; private set; } = false;
		internal EnemyController Controller { get; private set; } = new EnemyController();
		internal int Health { get; private set; } = 5;

		internal EnemyManager m_EnemyManager;

		protected override void OnCreate()
		{
			m_EnemyManager = FindEntityByName("EnemyManager").As<EnemyManager>();

			Controller.Init(this);
			m_Animator.Init(this);

			OnCollisionBegin2D += OnCollision;
			OnTriggerBegin2D += OnTriggerBegin;

			Log.Info("Enemy spawned!");
		}

		protected override void OnUpdate(float ts)
		{
			Controller.OnUpdate(ts);

			if (IsShooting)
			{
				if (m_ShootingTimer > m_ShootingCooldown)
				{
					m_ShootingTimer = 0.0f;
					SpawnBullet();
				}

				m_ShootingTimer += ts;
			}
			else
			{
				m_ShootingTimer = 0.2f; // For faster reaction
			}

			m_Animator.OnUpdate(ts);
		}

		internal void StartShooting()
		{
			IsShooting = true;
		}

		internal void StopShooting()
		{
			IsShooting = false;
		}

		private void SpawnBullet()
		{
			Vector2 direction = new Vector2(Mathf.Sign(Transform.Scale.X), 0.0f);

			Vector3 translation = Transform.Translation;
			translation += new Vector3(direction.X * m_OffsetX, m_OffsetY, 0);

			Bullet bullet = Instantiate(m_BulletPrefab, translation).As<Bullet>();
			bullet.SetDirection(direction);
		}

		private void OnCollision(Entity other)
		{
			switch (other.Name)
			{
				case "Hitbox-Horizontal":
					Controller.OnCollision(other);
					break;
				case "Bullet":
					Log.Info("HIT!");
					--Health;
					break;
			}
		}

		private void OnTriggerBegin(Entity other)
		{
			switch (other.Name)
			{
				case "Hitbox-Trigger":
					Controller.OnCollision(other);
					break;
				case "Bullet":
					Log.Info("HIT!");
					--Health;
					break;
			}
		}
	}
}
