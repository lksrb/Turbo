using Turbo;

namespace Mystery
{
	enum BallThrowState : uint
	{
		ReadyToGrab = 0,
		Grabbed,
		ThrowExhaust
	}

	internal class EnemyBallThrow : Layer<Enemy, EnemyEvent>
	{
		BallThrowState m_CurrentState = BallThrowState.ReadyToGrab;
		BouncyBall m_BouncyBall;
		RigidbodyComponent m_Rigidbody;

		Timer m_ThrowTimer = new Timer(1.0f);
		Timer m_ThrowExhaustTimer = new Timer(0.5f);

		protected override void OnAttach()
		{
			m_BouncyBall = m_Entity.FindEntityByName("BouncyBall").As<BouncyBall>();
			m_Rigidbody = m_Entity.GetComponent<RigidbodyComponent>();
		}

		protected override void OnUpdate()
		{
			Vector3 forwardDirection = new Quaternion(m_Entity.Transform.Rotation) * Vector3.Forward;
			forwardDirection.Y = 0.0f;
			forwardDirection.Normalize();

			// Pick up bouncy ball
			if (m_CurrentState == BallThrowState.ReadyToGrab)
			{
				var bouncyBall = m_BouncyBall.Transform.Translation;
				var enemy = m_Entity.Transform.Translation;

				if (CollisionTest.IsInCircle(bouncyBall, enemy, m_Entity.PickUpRadius))
				{
					if (m_BouncyBall.SetHolder(m_Entity))
					{
						ChangeState(BallThrowState.Grabbed);
					}
				}
			} 
			else if (m_CurrentState == BallThrowState.Grabbed)
			{
				var rb = m_BouncyBall.GetComponent<RigidbodyComponent>();
				rb.Position = m_Rigidbody.Position + forwardDirection * 2.0f + Vector3.Up * 0.5f;
				rb.Rotation = m_Rigidbody.Rotation;
				rb.LinearVelocity = Vector3.Zero;
				rb.AngularVelocity = Vector3.Zero;
				if (m_ThrowTimer)
				{
					rb.AddForce(forwardDirection * 50.0f, ForceMode.Impulse);
					m_BouncyBall.ReleaseFromHolder(m_Entity);
					ChangeState(BallThrowState.ThrowExhaust);
				}
			}
			else if (m_CurrentState == BallThrowState.ThrowExhaust) // Cooldown for picking again
			{
				if (m_ThrowExhaustTimer)
				{
					ChangeState(BallThrowState.ReadyToGrab);
				}
			}
		}

		protected override void OnEvent(EnemyEvent layerEvent)
		{
		}

		private void ChangeState(BallThrowState state)
		{
			if (m_CurrentState == state)
				return;

			m_CurrentState = state;

			if(m_CurrentState == BallThrowState.Grabbed)
			{
				Emit(EnemyEvent.BallGrabbed);
			} else if(m_CurrentState == BallThrowState.ThrowExhaust)
			{
				Emit(EnemyEvent.ChaseBall);
			}
		}
	}
}
