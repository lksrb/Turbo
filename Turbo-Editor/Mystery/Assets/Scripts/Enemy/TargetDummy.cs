using Turbo;

namespace Mystery
{
	internal class TargetDummy : Entity
	{
		public float Speed = 0.0f;

		Entity m_Player;
		RigidbodyComponent m_Rigidbody;

		Timer m_RecoveryTimer;
		bool m_Hit = false;

		Vector3 m_Velocity = Vector3.Zero;

		protected override void OnCreate()
		{
			m_Player = FindEntityByName("Player");
			m_Rigidbody = GetComponent<RigidbodyComponent>();
			m_RecoveryTimer = new Timer(5.0f);

			OnCollisionBegin += OnHit;
		}

		protected override void OnUpdate()
		{
			if (m_Hit)
			{
				if (m_RecoveryTimer)
				{
					m_Hit = false;
				}

				m_Rigidbody.LinearVelocity = Vector3.Lerp(m_Rigidbody.LinearVelocity, new Vector3(0, 0, 0), Frame.TimeStep);

				return;
			}

			Vector3 direction = m_Player.Transform.Translation - m_Rigidbody.Position;
			direction.Normalize();
			m_Velocity = direction * Speed;

			m_Rigidbody.LinearVelocity = Vector3.Lerp(m_Rigidbody.LinearVelocity, m_Velocity, Frame.TimeStep * Speed);
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
