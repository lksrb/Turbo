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
		private bool m_BulletShot = false;
		private readonly float m_ShootCooldowm = 0.7f;
		private float m_ShootCooldowmTimer = 0.0f;

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

			if (m_BulletShot && (m_ShootCooldowmTimer += ts) > m_ShootCooldowm)
			{
				m_ShootCooldowmTimer = 0.0f;
				m_BulletShot = false;
			}
		}

		private void ShootBullet(Vector2 direction)
		{
			Vector3 translation = m_Gun.Transform.Translation;
			translation.XY += direction * 1.1f;
			translation.Y += 0.1f;
			translation.Z = 0.5f;
			Bullet bullet = m_Player.InstantiateChild(m_BulletPrefab, translation).As<Bullet>();
			bullet.SetDirection(direction);
		}

		internal void Shoot()
		{
			if (m_BulletShot)
				return;

			for (int i = -4; i <= 4; i++)
			{
				Vector2 direction = m_ShootDirection;

				float angle = (Mathf.PI / 64) * (float)m_Random.NextDouble() * i;

				float xRotated = direction.X * Mathf.Cos(angle) - direction.Y * Mathf.Sin(angle);
				float yRotated = direction.X * Mathf.Sin(angle) + direction.Y * Mathf.Cos(angle);

				direction = new Vector2(xRotated, yRotated);

				direction *= Mathf.Clamp((float)m_Random.NextDouble() * 1.4f, 0.9f, 1.4f);

				ShootBullet(direction);
			}

			m_BulletShot = true;
		}
		private void MouseFollowCrosshair(float ts)
		{
			var translation = m_Crosshair.Transform.Translation;
			translation.XY = m_WorldMousePosition;
			m_Crosshair.Transform.Translation = translation;
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

			/*if (rotation.Z > 0.0 && rotation.Z < Mathf.PI)
			{
				var translation = m_Gun.Transform.Translation;
				translation.Z = 0.9f;
				m_Gun.Transform.Translation = translation;
			}
*/

			m_Gun.Transform.Scale = scale;
			m_Gun.Transform.Rotation = rotation;
			//m_Gun.Transform.Rotation = Mathf.Lerp(m_Gun.Transform.Rotation, rotation, 10.0f * ts);
		}

		private void FollowPlayer(float ts)
		{
			Vector3 translation = m_Player.Transform.Translation;
			Vector3 rotation = m_Gun.Transform.Rotation;

			if (rotation.Z > Mathf.PI / 2 && rotation.Z < Mathf.PI * 1.5f)
			{
				translation.X -= (m_GunInitialPos.X - m_PlayerInitialInitialPos.X);
			}
			else
			{
				translation.X += m_GunInitialPos.X - m_PlayerInitialInitialPos.X;
			}

			translation.Y += (m_GunInitialPos.Y - m_PlayerInitialInitialPos.Y);

			if (m_BulletShot)
			{
				translation.XY -= (m_ShootDirection * 0.7f) * (m_ShootCooldowm - m_ShootCooldowmTimer);
			}

			m_Gun.Transform.Translation = Mathf.Lerp(m_Gun.Transform.Translation, translation, 20.0f * ts);
		}
	}
}
