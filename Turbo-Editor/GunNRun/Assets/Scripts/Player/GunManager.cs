using System;
using System.Runtime.CompilerServices;
using Turbo;

namespace GunNRun
{
	internal class GunManager
	{
		private Player m_Player;
		private Vector2 m_PlayerInitialInitialPos;
		
		// Gun
		private Entity m_Gun;
		private Vector2 m_GunInitialPos;

		// Bullet
		private readonly string m_BulletPrefab = "Assets/Prefabs/Bullet.tprefab";
		private Vector2 m_ShootDirection = Vector2.Zero;

		// Cursor
		private Entity m_Crosshair;
		private Vector3 m_WorldMousePosition;
		private Random m_Random = new Random();

		internal void Init(Player player)
		{
			m_Player = player;
			m_PlayerInitialInitialPos = new Vector2(m_Player.Transform.Translation);

			m_Gun = m_Player.FindEntityByName("Gun");
			m_GunInitialPos = new Vector2(m_Gun.Transform.Translation);

			m_Crosshair = m_Player.FindEntityByName("Crosshair");

			// First one is slow
			m_Random.NextDouble();
		}

		internal void OnUpdate(float ts)
		{
			m_WorldMousePosition = Camera.ScreenToWorldPosition(Input.MousePosition);

			FollowPlayer(ts);
			MouseFollowCrosshair(ts);
			RotateToCrosshair(ts);
		}

		private void ShootBullet(Vector2 direction)
		{
			Vector3 translation = m_Gun.Transform.Translation;
			translation += direction * 0.7f;
			Bullet bullet = m_Player.Instantiate(m_BulletPrefab, translation).As<Bullet>();
			bullet.SetDirection(direction);
		}

		internal void Shoot()
		{
			for (int i = -2; i <= 2; i++)
			{
				Vector2 direction = m_ShootDirection;

				float angle = (Mathf.PI / 8) * i;

				float xRotated = direction.X * Mathf.Cos(angle) - direction.Y * Mathf.Sin(angle);
				float yRotated = direction.X * Mathf.Sin(angle) + direction.Y * Mathf.Cos(angle);

				direction = new Vector2(xRotated, yRotated);

				ShootBullet(direction);
			}

		}
		private void MouseFollowCrosshair(float ts)
		{
			m_Crosshair.Transform.Translation = m_WorldMousePosition;
		}

		private void RotateToCrosshair(float ts)
		{
			Vector3 playerPos = m_Player.Transform.Translation;

			m_ShootDirection = m_Crosshair.Transform.Translation - playerPos;
			m_ShootDirection.Normalize();

			float angle = Mathf.Atan(m_ShootDirection.Y / m_ShootDirection.X); // [-90,90]

			Vector3 scale = m_Gun.Transform.Scale;
			Vector3 rotation = m_Gun.Transform.Rotation;

			if (m_ShootDirection.X >= 0.0f)
			{
				//rotation.X = 0;
				scale.Y = Mathf.Abs(scale.X);
				rotation.Z = angle;
			}
			else
			{
				//rotation.X = Mathf.PI;
				scale.Y = -Mathf.Abs(scale.X);
				rotation.Z = angle + Mathf.PI;
			}

			m_Gun.Transform.Scale = scale;
			m_Gun.Transform.Rotation = rotation;
			//m_Gun.Transform.Rotation = Mathf.Lerp(m_Gun.Transform.Rotation, rotation, 10.0f * ts);
		}

		private void FollowPlayer(float ts)
		{
			Vector3 translation = m_Player.Transform.Translation;
			translation.X += (m_GunInitialPos.X - m_PlayerInitialInitialPos.X) * Mathf.Sign(m_Player.Transform.Scale.X);
			translation.Y += (m_GunInitialPos.Y - m_PlayerInitialInitialPos.Y);

			// TODO: Lerp?
			m_Gun.Transform.Translation = translation;
			//Mathf.Lerp(m_Gun.Transform.Translation, translation, 10.0f * ts)
			return;
		}
	}
}
