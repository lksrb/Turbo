using Turbo;

namespace Mystery
{
	internal class EnemyExhaustedByThrow : IEnemyState
	{
		Enemy m_Enemy;
		RigidbodyComponent m_Rigidbody;
		BouncyBall m_BouncyBall;
		Player m_Player;

		Timer m_ExhaustTimer;

		public EnemyExhaustedByThrow(Enemy enemy)
		{
			m_Enemy = enemy;

			m_Rigidbody = m_Enemy.GetComponent<RigidbodyComponent>();
			m_BouncyBall = m_Enemy.FindEntityByName("BouncyBall").As<BouncyBall>();
			m_Player = m_Enemy.FindEntityByName("Player").As<Player>();

			m_ExhaustTimer = new Timer(2.0f, false, false);
		}

		public void Enter()
		{
			m_ExhaustTimer.Reset();
			m_ExhaustTimer.Start();
		}

		public void OnUpdate()
		{
			if(m_ExhaustTimer)
			{
				m_Enemy.ChangeState(EnemyState.ChaseBall);
			}
		}
	}
}
