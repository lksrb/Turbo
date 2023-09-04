using Turbo;

namespace Mystery
{
	internal class EnemyRunAwayState : IEnemyState
	{
		Enemy m_Enemy;
		RigidbodyComponent m_Rigidbody;
		Player m_Player;
		BouncyBall m_BouncyBall;

		public EnemyRunAwayState(Enemy enemy)
		{
			m_Enemy = enemy;

			m_Rigidbody = m_Enemy.GetComponent<RigidbodyComponent>();
			m_Player = m_Enemy.FindEntityByName("Player").As<Player>();
			m_BouncyBall = m_Enemy.FindEntityByName("BouncyBall").As<BouncyBall>();
		}

		public void Enter()
		{
		}

		public void OnUpdate()
		{
			Vector3 distance = m_Rigidbody.Position - m_Player.Transform.Translation;
			Vector3 direction = distance;
			direction.Y = 0.0f;
			direction.Normalize();

			// Movement
			m_Rigidbody.LinearVelocity = direction * m_Enemy.Speed;

			// Rotation
			{
				Quaternion targetRotation = Quaternion.LookAt(direction, Vector3.Up);

				// Smoothly change rotation according to direction towards target
				m_Rigidbody.Rotation = Quaternion.Slerp(m_Rigidbody.Rotation, targetRotation, Frame.TimeStep * 13.0f);
			}

			if(m_BouncyBall.Owner == m_Player)
			{
				if (distance.Length() < 25.0f)
				{
					m_Enemy.ChangeState(EnemyState.Defend);

				}
			} 
			else if(m_BouncyBall.Owner == null)
			{
				m_Enemy.ChangeState(EnemyState.ChaseBall);
			}
		}
	}
}
