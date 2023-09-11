using Turbo;

namespace Mystery
{
	internal class EnemyDeadState : EnemyStateBase
	{
		Timer m_DeathTimer;

		public EnemyDeadState(Enemy enemy, Player player) : base(enemy, player)
		{
			m_DeathTimer = new Timer(2.0f);
		}

		internal override void Enter()
		{
			Log.Info("Enemy dead");
		}

		internal override void OnUpdate()
		{
			if(m_DeathTimer)
			{
				// TODO: Explosion

				Scene.DestroyEntity(m_Enemy);
			}
		}

		protected override void OnCollisionBegin(Entity entity)
		{
		}

		protected override void OnCollisionEnd(Entity entity)
		{
		}
	}
}
