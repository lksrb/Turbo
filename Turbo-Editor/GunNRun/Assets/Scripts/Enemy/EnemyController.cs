using System;
using Turbo;

namespace GunNRun
{
	internal enum Direction
	{
		Right = 1,
		Left = -1,
	}

	internal class EnemyController
	{
		private Enemy m_Enemy;
		private Rigidbody2DComponent m_Rigidbody2D;
		private TransformComponent m_Transform;
		private Vector2 m_Velocity = Vector2.Zero;
		private Random m_Random = new Random();
		private float m_NextMoveCurrentTime;
		private readonly float m_MinTime = 2.0f;
		private Entity m_Player;

		private float m_Speed = 5.0f;
		private Direction m_Direction = Direction.Right;

		private uint m_Moving = 1;

		internal Vector2 Velocity { get => m_Velocity; }

		public void Init(Enemy enemy)
		{
			m_Enemy = enemy;
			m_Player = m_Enemy.FindEntityByName("Player");
			m_Rigidbody2D = m_Enemy.GetComponent<Rigidbody2DComponent>();
			m_Transform = m_Enemy.Transform;
			m_NextMoveCurrentTime = (float)m_Random.NextDouble() + m_MinTime;
		}

		public void OnUpdate(float ts)
		{
			if (m_Enemy.Health <= 0)
			{
				m_Rigidbody2D.IsEnabled = false;
				return;
			}

			m_Velocity = m_Rigidbody2D.Velocity;

			// Enemy AI
			{
				// Direction
				Vector3 dist = m_Transform.Translation - m_Player.Transform.Translation;

				if (Mathf.Abs(dist.X) < 15.0f && Mathf.Abs(dist.Y) < 1.0f)
				{
					Direction dir = dist.X < 0.0f ? Direction.Right : Direction.Left;
					ChangeDirection(dir);

					// Shoot player
					m_Enemy.StartShooting();
				}
				else
				{
					m_Enemy.StopShooting();
				}

				// Velocity
			}

			// When shooting, stop moving
			if(m_Enemy.IsShooting)
			{
				m_Moving = 0;
			}

			m_Velocity.X = m_Speed * (int)m_Direction * m_Moving;
			
			// Set final velocity
			m_Rigidbody2D.Velocity = m_Velocity;
		}

		void ChangeDirection(Direction direction)
		{
			if (m_Direction == direction)
				return;

			m_Direction = direction;

			Vector3 scale = m_Transform.Scale;
			scale.X *= -1;
			m_Transform.Scale = scale;
		}

		public void OnCollision(Entity hitbox)
		{
			if (m_Velocity.X != 0)
			{
				m_Moving = 0;
			}

			//m_Speed *= -1;

			//m_Rigidbody2D.Velocity = new Vector2(0, m_Rigidbody2D.Velocity.Y);
		}
	}
}
