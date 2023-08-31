using Turbo;

namespace Mystery
{
	enum EnemyPickState : uint
	{
		BallReadyToPick = 0,
		BallPicked,
		ThrowExhaust,
	}

	internal class Enemy : Entity
	{
		public float Speed = 0.0f;
		public bool FollowPlayer = false;
		public float PickUpRadius = 4.0f;

		EnemyPickState m_CurrentPickState = EnemyPickState.BallReadyToPick;

		Entity m_Player;
		RigidbodyComponent m_Rigidbody;

		Timer m_RecoveryTimer;
		bool m_Hit = false;

		Vector3 m_Velocity = Vector3.Zero;

		Entity m_PickedUpBall;
		BouncyBall m_BouncyBall;

		Timer m_ThrowTimer = new Timer(1.0f);
		Timer m_ThrowExhaustTimer = new Timer(0.5f);

		Vector3 m_ForwardDirection = Vector3.Zero;

		protected override void OnCreate()
		{
			m_Player = FindEntityByName("Player");
			m_BouncyBall = FindEntityByName("BouncyBall").As<BouncyBall>();
			m_Rigidbody = GetComponent<RigidbodyComponent>();
			m_RecoveryTimer = new Timer(5.0f);

			OnCollisionBegin += OnHit;
		}

		protected override void OnUpdate()
		{
			m_ForwardDirection = new Quaternion(Transform.Rotation) * Vector3.Forward;
			m_ForwardDirection.Y = 0.0f;
			m_ForwardDirection.Normalize();

			OnMovementUpdate();

			// Pick up bouncy ball
			if(m_CurrentPickState == EnemyPickState.BallReadyToPick)
			{
				if (CollisionTest.IsInCircle(m_BouncyBall.Transform.Translation, Transform.Translation, PickUpRadius))
				{
					if (m_BouncyBall.SetHolder(this))
					{
						m_PickedUpBall = m_BouncyBall;
						ChangeState(EnemyPickState.BallPicked);
					}
				}
			}

			if(m_CurrentPickState == EnemyPickState.BallPicked)
			{
				var rb = m_PickedUpBall.GetComponent<RigidbodyComponent>();
				rb.Position = m_Rigidbody.Position + m_ForwardDirection * 2.0f + Vector3.Up * 0.5f;
				rb.Rotation = m_Rigidbody.Rotation;
				rb.LinearVelocity = Vector3.Zero;
				rb.AngularVelocity = Vector3.Zero;

				if (m_ThrowTimer)
				{
					rb.AddForce(m_ForwardDirection * 50.0f, ForceMode.Impulse);

					m_BouncyBall.ReleaseFromHolder(this);
					m_PickedUpBall = null;

					ChangeState(EnemyPickState.ThrowExhaust);
				}
			}

			// Cooldown for picking again
			if(m_CurrentPickState == EnemyPickState.ThrowExhaust)
			{
				if(m_ThrowExhaustTimer)
				{
					ChangeState(EnemyPickState.BallReadyToPick);
				}
			}
		}

		private void OnMovementUpdate()
		{
			if (m_Hit)
			{
				if (m_RecoveryTimer)
				{
					m_Hit = false;
				}

				m_Rigidbody.LinearVelocity = Vector3.Lerp(m_Rigidbody.LinearVelocity, Vector3.Zero, Frame.TimeStep);

				return;
			}

			// Rotate towards player
			{
				Vector3 direction = Transform.Translation - m_Player.Transform.Translation;
				direction.Normalize();

				direction.Y = 0.0f;
				m_Rigidbody.Rotation = Quaternion.LookAt(direction, Vector3.Up);
			}

			// Follow player
			if (FollowPlayer)
			{
				Vector3 direction = m_Player.Transform.Translation - m_Rigidbody.Position;
				direction.Normalize();
				m_Velocity = direction * Speed;

				m_Rigidbody.LinearVelocity = Vector3.Lerp(m_Rigidbody.LinearVelocity, m_Velocity, Frame.TimeStep * Speed);
			}

			DebugRenderer.DrawCircle(Transform.Translation, Vector3.Right * Mathf.HalfPI, PickUpRadius, Color.Green);
		}

		void ChangeState(EnemyPickState state)
		{
			if (m_CurrentPickState == state)
				return;

			m_CurrentPickState = state;
		}

		void OnHit(Entity entity)
		{
			if (entity.Name == "BouncyBall")
			{
				if (entity.GetComponent<RigidbodyComponent>().LinearVelocity.Length() > 10.0f)
				{
					m_Hit = true;
					m_RecoveryTimer.Reset();
				}

			}
		}
	}
}
