using Turbo;

namespace GunNRun
{
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

		private AudioSourceComponent m_MachineGunShootAudio;
		private Timer m_DeltaShootTimer = new Timer(0.05f);
		private Timer m_TimeBetweenBurstShot = new Timer(1.7f, false);
		private bool m_CanShoot = false;
		private bool m_StartBurst = true;

		internal ShooterGun(ShooterEnemy enemy, Player player)
		{
			m_ShooterEnemy = enemy;
			m_Player = player;
			m_Gun = m_ShooterEnemy.GetChildren()[0];

			m_GunShooterOffset = new Vector2(-0.1f, -0.24f);

			m_MachineGunShootAudio = m_Gun.GetComponent<AudioSourceComponent>();
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
				m_MachineGunShootAudio.Play();
			}
			else
			{
				m_MachineGunShootAudio.Stop();
			}

			// Set modified variables
			m_Gun.Transform.Translation = m_Translation;
			m_Gun.Transform.Rotation = m_Rotation;
			m_Gun.Transform.Scale = m_Scale;
		}

		internal void StartShooting()
		{
			if (!m_CanShoot)
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
}
