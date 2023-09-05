using Turbo;

namespace Mystery
{
	internal class EnemyChaseBallState : IEnemyState
	{
		Enemy m_Enemy;
		RigidbodyComponent m_Rigidbody;
		BouncyBall m_BouncyBall;
		AABB m_GrabCollider;

		public EnemyChaseBallState(Enemy enemy)
		{
			m_Enemy = enemy;

			m_Rigidbody = m_Enemy.GetComponent<RigidbodyComponent>();
			m_BouncyBall = m_Enemy.FindEntityByName("BouncyBall").As<BouncyBall>();

			m_GrabCollider = new AABB(Vector3.Zero, new Vector3(1.5f, 3.0f, 1.5f));
		}

		public void Enter()
		{
		}

		public void OnUpdate()
		{
			Vector3 direction = m_BouncyBall.Transform.Translation - m_Rigidbody.Position;
			direction.Y = 0.0f;
			direction.Normalize();
			
			// Update grab collider
			m_GrabCollider.Center = m_Enemy.Transform.Translation + m_Enemy.Forward * 2.0f;

			// Movement
			{
				m_Rigidbody.LinearVelocity = direction * m_Enemy.Speed;
			}

			// Rotation
			{
				Quaternion targetRotation = Quaternion.LookAt(direction, Vector3.Up);

				// Smoothly change rotation according to direction towards target
				m_Rigidbody.Rotation = Quaternion.Slerp(m_Rigidbody.Rotation, targetRotation, Frame.TimeStep * 13.0f);
			}

			if (m_GrabCollider.Contains(m_BouncyBall.Transform.Translation))
			{
				m_Enemy.ChangeState(EnemyState.Attack);
			}

			DebugRenderer.DrawBox(m_GrabCollider.Center, Vector3.Zero, m_GrabCollider.Size, Color.Green);
		}

		public void OnPlayerEvent(PlayerEvent playerEvent)
		{
			switch (playerEvent)
			{
				case PlayerEvent.BallGrabbed:
					m_Enemy.ChangeState(EnemyState.RunAway);
					break;
				case PlayerEvent.BallThrew:
					break;
			}
		}
	}
}
