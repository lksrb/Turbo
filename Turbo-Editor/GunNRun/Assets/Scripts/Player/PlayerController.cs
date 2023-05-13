using System;
using Turbo;

namespace GunNRun
{
	internal class PlayerController
	{
		private Player m_Player;
		private Rigidbody2DComponent m_Rigidbody2D;
		private Vector2 m_Velocity = Vector2.Zero;

		internal Vector2 Velocity => m_Velocity;

		internal void Init(Player player)
		{
			m_Player = player;
			m_Rigidbody2D = m_Player.GetComponent<Rigidbody2DComponent>();
		}

		internal void OnUpdate()
		{
			m_Velocity = Vector2.Zero;

			if (m_Player.Input.IsLeftKeyPressed)
			{
				Vector3 rotatedScale = m_Player.Transform.Scale;
				rotatedScale.X = -Mathf.Abs(rotatedScale.X);
				m_Player.Transform.Scale = rotatedScale;

				m_Velocity.X = -m_Player.Speed;
			}

			if (m_Player.Input.IsRightKeyPressed)
			{
				m_Player.Transform.Scale = Mathf.Abs(m_Player.Transform.Scale);

				m_Velocity.X = m_Player.Speed;
			}

			if (m_Player.Input.IsUpKeyPressed)
			{
				m_Velocity.Y = m_Player.Speed;
			}

			if (m_Player.Input.IsDownKeyPressed)
			{
				m_Velocity.Y = -m_Player.Speed;
			}

			if(m_Velocity != m_Rigidbody2D.Velocity)
			{
				ChangeDirection();
			}

			m_Rigidbody2D.Velocity = m_Velocity;

			//m_Rigidbody2D.Velocity = m_Velocity.Length != 0 ? Mathf.Lerp(m_Rigidbody2D.Velocity, m_Velocity * m_Player.Speed, ts * 10.0f) : m_Velocity;
		}

		private void ChangeDirection()
		{
			Vector3 scale = m_Player.Transform.Scale;
			scale.X *= m_Player.Velocity.X != 0.0f ? Mathf.Sign(m_Player.Velocity.X) : 1.0f;
			m_Player.Transform.Scale = scale;
		}
	}
}
