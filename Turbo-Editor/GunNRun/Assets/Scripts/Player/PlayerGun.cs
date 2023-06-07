using Turbo;

namespace GunNRun
{
	internal class PlayerGun
	{
		private Entity m_Gun, m_Player;
		private Vector2 m_PlayerInitialInitialPos, m_GunInitialPos;

		private Vector2 m_ShootDirection = Vector2.Zero;
		private Entity m_Crosshair; // Crosshair to follow

		private bool m_IsGunAnimating = false;
		private Timer m_ShootCoolDown = new Timer(1.1f); // Audio length

		internal bool BulletShot { get; private set; } = false;
		internal bool IsReady => !m_IsGunAnimating;

		internal PlayerGun(Player player)
		{
			m_Player = player;
			m_Gun = m_Player.FindEntityByName("Gun");
			m_Crosshair = m_Player.FindEntityByName("Crosshair");

			m_PlayerInitialInitialPos = m_Player.Transform.Translation;
			m_GunInitialPos = m_Gun.Transform.Translation;
		}

		internal void OnUpdate()
		{
			FollowPlayer();
			RotateToCrosshair();

			if (m_IsGunAnimating && m_ShootCoolDown)
			{
				m_IsGunAnimating = false;
			}

			BulletShot = false;
		}

		internal void Shoot()
		{
			for (int i = -2; i <= 2; i++)
			{
				Vector2 direction = m_ShootDirection;

				float angle = (Mathf.PI / 64.0f) * Random.Float() * i;

				float xRotated = direction.X * Mathf.Cos(angle) - direction.Y * Mathf.Sin(angle);
				float yRotated = direction.X * Mathf.Sin(angle) + direction.Y * Mathf.Cos(angle);

				direction = new Vector2(xRotated, yRotated);
				direction *= 0.8f;

				ShootBullet(direction);
			}

			SoundEffect.Play(Effect.Shotgun, m_Gun.Transform.Translation);

			BulletShot = true;
			m_IsGunAnimating = true;
		}

		internal void Hide()
		{
			var src = m_Gun.GetComponent<SpriteRendererComponent>();
			var color = src.SpriteColor;
			color.A = 0.0f;
			src.SpriteColor = color;
		}

		private void ShootBullet(Vector2 direction)
		{
			Vector3 translation = m_Gun.Transform.Translation;
			translation.XY += direction * 1.1f;
			translation.Y += 0.1f;
			translation.Z = 0.5f;

			Bullet.Create(m_Player, translation, direction, 25.0f, 0.7f, 2.0f);
		}

		private void RotateToCrosshair()
		{
			m_ShootDirection = m_Crosshair.Transform.Translation - m_Player.Transform.Translation;
			m_ShootDirection.Normalize();

			float angle = Mathf.Atan(m_ShootDirection.Y / m_ShootDirection.X); // [-90,90]

			Vector3 scale = m_Gun.Transform.Scale;
			Vector3 rotation = m_Gun.Transform.Rotation;

			if (m_ShootDirection.X >= 0.0f)
			{
				scale.Y = Mathf.Abs(scale.Y);
				rotation.Z = angle;
			}
			else
			{
				scale.Y = -Mathf.Abs(scale.Y);
				rotation.Z = angle + Mathf.PI;
			}

			m_Gun.Transform.Scale = scale;
			m_Gun.Transform.Rotation = rotation;
		}

		private void FollowPlayer()
		{
			Vector3 translation = m_Player.Transform.Translation;
			Vector3 rotation = m_Gun.Transform.Rotation;

			if (rotation.Z > Mathf.PI / 2.0f && rotation.Z < Mathf.PI * 1.5f)
			{
				translation.X -= m_GunInitialPos.X - m_PlayerInitialInitialPos.X;
			}
			else
			{
				translation.X += m_GunInitialPos.X - m_PlayerInitialInitialPos.X;
			}

			translation.Y += (m_GunInitialPos.Y - m_PlayerInitialInitialPos.Y);

			if (m_IsGunAnimating)
			{
				translation.XY -= (m_ShootDirection * 0.25f) * m_ShootCoolDown.Delta;
			}

			m_Gun.Transform.Translation = translation;
		}
	}
}
