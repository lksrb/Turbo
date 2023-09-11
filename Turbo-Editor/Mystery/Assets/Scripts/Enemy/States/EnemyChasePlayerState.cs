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
			Vector3 distance = m_Player.Transform.Translation - m_Enemy.Transform.Translation;
			distance.Normalize();

			// Run straight forward
			Vector3 linearVelocity = m_Rigidbody.LinearVelocity;
			linearVelocity = 5.0f * distance;
			linearVelocity.Y = m_Rigidbody.LinearVelocity.Y;
			m_Rigidbody.LinearVelocity = linearVelocity;
		}

		protected override void OnCollisionBegin(Entity entity)
		{
			if(entity.Name == "BouncyBall")
			{
				if(entity.As<BouncyBall>().CanDamage)
				{
					ChangeState(EnemyState.Stunned);
				}
			}
		}

		protected override void OnCollisionEnd(Entity entity)
		{
		}
	}
}
