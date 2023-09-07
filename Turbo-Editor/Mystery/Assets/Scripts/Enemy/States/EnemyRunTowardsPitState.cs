using Turbo;

namespace Mystery
{
	internal class EnemyRunTowardsPitState : EnemyStateBase
	{
		Entity m_Pit;

		internal EnemyRunTowardsPitState(Enemy enemy, Player player) : base(enemy, player)
		{
			m_Pit = FindEntityByName("PitGround");
		}

		internal override void Enter()
		{
			Log.Info("Enemy run towards pit!");
		}

		internal override void OnUpdate()
		{
			float distance = m_Pit.Transform.Translation.Z - m_Enemy.Transform.Translation.Z;

			// Run straight forward
			Vector3 linearVelocity = m_Rigidbody.LinearVelocity;
			linearVelocity.Z = 5.0f * Mathf.Sign(distance);
			m_Rigidbody.LinearVelocity = linearVelocity;

			// Until the pit
			if(distance < 3.0f)
			{
				ChangeState(EnemyState.Lifeless);
			}
		}

		internal override void OnPlayerEvent(PlayerEvent playerEvent)
		{
		}

		protected override void OnCollisionBegin(Entity entity)
		{
			if(entity.Name == "BouncyBall")
			{
				ChangeState(EnemyState.Stunned);
			}
		}

		protected override void OnCollisionEnd(Entity entity)
		{
		}
	}
}
