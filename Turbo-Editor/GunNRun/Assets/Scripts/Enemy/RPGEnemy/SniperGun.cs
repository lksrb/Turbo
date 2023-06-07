using Turbo;

namespace GunNRun
{
	internal class SniperGun
	{
		private SniperEnemy m_SniperEnemy;
		private Entity m_Player;

		private Entity m_Gun;
		private Vector2 m_EnemyGunOffset;

		private Vector2 m_ShootDirection;
		private Vector3 m_Translation;
		private Vector3 m_Scale;
		private Vector3 m_Rotation;

		internal Vector3 Translation => m_Translation;

		internal SniperGun(SniperEnemy enemy)
		{
			m_SniperEnemy = enemy;

			m_Gun = enemy.FindEntityByName("SniperGun");
			m_EnemyGunOffset = new Vector2(0.0f, -0.2f);

			m_Player = enemy.FindEntityByName("Player");
		}

		internal void OnUpdate()
		{
			// Reducing the amount of dll calls
			m_Translation = m_SniperEnemy.Transform.Translation + m_EnemyGunOffset;
			m_Scale = m_Gun.Transform.Scale;

			m_Gun.Transform.Translation = m_Translation;
			m_Translation.Z = 1.0f;

			OnRotateToPlayer();

			// Set modified variables
			m_Gun.Transform.Translation = m_Translation;
			m_Gun.Transform.Rotation = m_Rotation;
			m_Gun.Transform.Scale = m_Scale;
		}

		internal void Shoot()
		{
			ShootBullet(m_ShootDirection);
		}

		private void OnRotateToPlayer()
		{
			m_ShootDirection = m_Player.Transform.Translation - m_Translation;
			m_ShootDirection.Normalize();

			float angle = Mathf.Atan(m_ShootDirection.Y / m_ShootDirection.X); // [-90,90]

			m_Scale.Y = m_ShootDirection.X >= 0.0f ? Mathf.Abs(m_Scale.Y) : -Mathf.Abs(m_Scale.Y);
			m_Rotation.Z = m_ShootDirection.X >= 0.0f ? angle : angle + Mathf.PI;
		}

		private void ShootBullet(Vector2 direction)
		{
			Vector3 translation = m_Gun.Transform.Translation;
			translation.XY += direction * 0.85f;
			//translation.Y += 0.1f;
			translation.Z = 0.5f;

			Bullet bullet = Bullet.Create(m_SniperEnemy, translation, direction, 35);
			bullet.GetComponent<SpriteRendererComponent>().SpriteColor = Color.Orange;
		}

	}
}
