using Turbo;

namespace Mystery
{
	internal class EnemyLifelessState : EnemyStateBase
	{
		public EnemyLifelessState(Enemy enemy, Player player) : base(enemy, player)
		{
		}

		internal override void Enter()
		{
			Log.Info("Enemy lifeless");
		}

		internal override void OnUpdate()
		{
		}

		internal override void OnPlayerEvent(PlayerEvent playerEvent)
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
