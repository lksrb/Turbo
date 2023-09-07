using Turbo;

namespace Mystery
{
	internal class EnemyChasePlayerState : EnemyStateBase
	{
		internal EnemyChasePlayerState(Enemy enemy, Player player) : base(enemy, player)
		{
		}

		internal override void Enter()
		{
			Log.Info("Enemy chase player!");
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
