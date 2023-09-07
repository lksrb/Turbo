using Turbo;

namespace Mystery
{
	internal class EnemyStunnedState : EnemyStateBase
	{
		public EnemyStunnedState(Enemy enemy, Player player) : base(enemy, player)
		{
		}

		internal override void Enter()
		{
			Log.Info("Enemy stunned!");
		}

		internal override void OnPlayerEvent(PlayerEvent playerEvent)
		{
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
