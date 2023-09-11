using Turbo;

namespace Mystery
{
	internal class EnemyDeadState : EnemyStateBase
	{
		public EnemyDeadState(Enemy enemy, Player player) : base(enemy, player)
		{
		}

		internal override void Enter()
		{
			Log.Info("Enemy dead");
		}

		internal override void OnUpdate()
		{
		}

		protected override void OnCollisionBegin(Entity entity)
		{
		}

		protected override void OnCollisionEnd(Entity entity)
		{
		}
	}
}
