using Turbo;

namespace Mystery
{
	internal class EnemyGrabAndThrowBallState : IEnemyState
	{
		Enemy m_Enemy;
		BouncyBall m_BouncyBall;
		RigidbodyComponent m_Rigidbody;
		Player m_Player;
		Timer m_ThrowTimer;

		internal EnemyGrabAndThrowBallState(Enemy enemy)
		{
			m_Enemy = enemy;
			m_BouncyBall = m_Enemy.FindEntityByName("BouncyBall").As<BouncyBall>();
			m_Player = m_Enemy.FindEntityByName("Player").As<Player>();
			m_Rigidbody = m_Enemy.GetComponent<RigidbodyComponent>();
		}

		public void Enter()
		{
			m_ThrowTimer = new Timer(Random.Float(0.5f, 2.0f), false, false);

			if (m_BouncyBall.SetOwner(m_Enemy))
			{
				// Ball is already holded by player or other enemy
				m_ThrowTimer.Start();
			} else
			{
				Log.Error("Ball already has a holder!");
			}
		}
		public void OnUpdate()
		{
			Vector3 direction = m_Player.Transform.Translation - m_Rigidbody.Position;
			direction.Y = 0.0f;
			direction.Normalize();

			var rb = m_BouncyBall.GetComponent<RigidbodyComponent>();
			rb.Position = m_Rigidbody.Position + m_Enemy.Forward * 2.0f + Vector3.Up * 0.5f;
			rb.Rotation = m_Rigidbody.Rotation;
			rb.LinearVelocity = Vector3.Zero;
			rb.AngularVelocity = Vector3.Zero;

			m_Rigidbody.LinearVelocity = Vector3.Lerp(m_Rigidbody.LinearVelocity, Vector3.Zero, Frame.TimeStep * 5.0f);

			// Rotation
			{
				Quaternion targetRotation = Quaternion.LookAt(direction, Vector3.Up);

				// Smoothly change rotation according to direction towards target
				m_Rigidbody.Rotation = Quaternion.Slerp(m_Rigidbody.Rotation, targetRotation, Frame.TimeStep * 13.0f);
			}

			// Take some time and then throw ball
			if(m_ThrowTimer)
			{
				rb.AddForce(m_Enemy.Forward * 50.0f, ForceMode.Impulse);
				m_BouncyBall.Release(m_Enemy);
				m_Enemy.ChangeState(EnemyState.ExhaustedByThrow);
			}
		}
	}
}
