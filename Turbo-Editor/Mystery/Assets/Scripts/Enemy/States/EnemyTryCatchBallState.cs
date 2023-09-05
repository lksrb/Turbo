using Turbo;

namespace Mystery
{
	internal class EnemyTryCatchBallState : IEnemyState
	{
		Enemy m_Enemy;
		BouncyBall m_BouncyBall;
		Player m_Player;

		internal EnemyTryCatchBallState(Enemy enemy)
		{
			m_Enemy = enemy;

			m_Player = m_Enemy.FindEntityByName("Player").As<Player>();
			m_BouncyBall = m_Enemy.FindEntityByName("BouncyBall").As<BouncyBall>();
		}

		public void Enter()
		{
		}

		public void OnUpdate()
		{
			Log.Info("CATCHING HOPEFULLY");
		}

		public void OnPlayerEvent(PlayerEvent playerEvent)
		{
		}
	}
}
