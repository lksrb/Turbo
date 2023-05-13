using System;
using System.Runtime.CompilerServices;
using Turbo;

namespace GunNRun
{
	internal class PlayerGun
	{
		private Player m_Player;
		private Vector2 m_PlayerInitialInitialPos;

		// Gun
		private Entity m_Gun;
		private Vector2 m_GunInitialPos;

		// Bullet
		private readonly string m_BulletPrefab = "Assets/Prefabs/Bullet.tprefab";
		private Vector2 m_ShootDirection = Vector2.Zero;
		private bool m_IsBulletAnimating = false;

		private Timer m_ShootCoolDown = new Timer(0.7f);
		internal bool BulletShot { get; private set; } = false;

		// Cursor
		private Entity m_Crosshair;
		private Vector3 m_WorldMousePosition;

		internal void Init(Player player)
		{
			m_Player = player;
			m_PlayerInitialInitialPos = new Vector2(m_Player.Transform.Translation);

			m_Gun = m_Player.FindEntityByName("Gun");
			m_GunInitialPos = new Vector2(m_Gun.Transform.Translation);

			m_Crosshair = m_Player.FindEntityByName("Crosshair");
		}

		internal void OnUpdate()
		{
			m_WorldMousePosition = Camera.ScreenToWorldPosition(Input.MousePosition);

			OnFollowPlayer();
			OnMouseFollowCrosshair();
			OnRotateToCrosshair();

			if (m_IsBulletAnimating && m_ShootCoolDown)
			{
				m_IsBulletAnimating = false;
			}

			BulletShot = false;
		}

		private void ShootBullet(Vector2 direction)
		{
			Vector3 translation = m_Gun.Transform.Translation;
			translation.XY += direction * 1.1f;
			translation.Y += 0.1f;
			translation.Z = 0.5f;
			Bullet bullet = m_Player.InstantiateChild(m_BulletPrefab, translation).As<Bullet>();
			bullet.Init(m_Player, direction);
		}

		internal void Shoot()
		{
			if (m_IsBulletAnimating)
				return;

			for (int i = -4; i <= 4; i++)
			{
				Vector2 direction = m_ShootDirection;

				float angle = (Mathf.PI / 64) * Random.Float() * i;

				float xRotated = direction.X * Mathf.Cos(angle) - direction.Y * Mathf.Sin(angle);
				float yRotated = direction.X * Mathf.Sin(angle) + direction.Y * Mathf.Cos(angle);

				direction = new Vector2(xRotated, yRotated);

				direction *= 0.8f;

				ShootBullet(direction);
			}

			BulletShot = true;

			m_IsBulletAnimating = true;
		}
		private void OnMouseFollowCrosshair()
		{
			var translation = m_Crosshair.Transform.Translation;
			translation.XY = m_WorldMousePosition;
			m_Crosshair.Transform.Translation = translation;
		}

		private void OnRotateToCrosshair()
		{
			Vector3 playerPos = m_Player.Transform.Translation;

			m_ShootDirection = m_Crosshair.Transform.Translation - playerPos;
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

		private void OnFollowPlayer()
		{
			Vector3 translation = m_Player.Transform.Translation;
			Vector3 rotation = m_Gun.Transform.Rotation;
			float lerpMagnifier = 20.0f;

			if (rotation.Z > Mathf.PI / 2 && rotation.Z < Mathf.PI * 1.5f)
			{
				translation.X -= m_GunInitialPos.X - m_PlayerInitialInitialPos.X;
			}
			else
			{
				translation.X += m_GunInitialPos.X - m_PlayerInitialInitialPos.X;
			}

			translation.Y += (m_GunInitialPos.Y - m_PlayerInitialInitialPos.Y);

			if (m_IsBulletAnimating)
			{
				translation.XY -= (m_ShootDirection * 0.7f) * m_ShootCoolDown.Delta;
				lerpMagnifier *= 2.0f;
			}

			m_Gun.Transform.Translation = Mathf.Lerp(m_Gun.Transform.Translation, translation, lerpMagnifier * Frame.TimeStep);
		}
	}
}
