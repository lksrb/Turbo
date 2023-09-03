using System;
using Turbo;

namespace Mystery
{
	internal class EnemyMovement : Layer<Enemy, EnemyEvent>
	{
		RigidbodyComponent m_Rigidbody;
		Player m_Player;
		bool m_Hit = false;
		Vector3 m_Velocity = Vector3.Zero;

		Timer m_RecoveryTimer;

		float m_Direction = 1.0f;

		Entity m_BouncyBall;

		Timer m_AfterPlayerThrowTimer = new Timer(1.0f, true, false);

		Vector3 m_TargetLocation = Vector3.Zero;

		EnemyTarget m_Target = EnemyTarget.TargetBall;
		MovementFlags m_Flags = MovementFlags.All;

		protected override void OnAttach()
		{
			m_Rigidbody = m_Entity.GetComponent<RigidbodyComponent>();
			m_Player = m_Entity.FindEntityByName("Player").As<Player>();
			m_BouncyBall = m_Entity.FindEntityByName("BouncyBall");
			m_RecoveryTimer = new Timer(5.0f);

			m_TargetLocation = m_BouncyBall.Transform.Translation;
		}

		protected override void OnUpdate()
		{
			m_TargetLocation = m_Target == EnemyTarget.TargetBall ? m_BouncyBall.Transform.Translation : m_Player.Transform.Translation;

			/*	if (m_Hit)
				{
					if (m_RecoveryTimer)
					{
						m_Hit = false;
					}

					m_Rigidbody.LinearVelocity = Vector3.Lerp(m_Rigidbody.LinearVelocity, Vector3.Zero, Frame.TimeStep);

					return;
				}
	*/

			// Rotate towards target
			if (m_Flags.HasFlag(MovementFlags.Rotate))
			{
				Vector3 direction = m_Entity.Transform.Translation - m_TargetLocation;
				direction.Normalize();
				direction.Y = 0.0f;

				Quaternion targetRotation = Quaternion.LookAt(new Vector3(direction.X, 0.0f, direction.Z), Vector3.Up);

				// Smoothly change rotation according to direction towards target
				m_Rigidbody.Rotation = Quaternion.Slerp(m_Rigidbody.Rotation, targetRotation, Frame.TimeStep * 13.0f);
			}

			// Follow target
			if (m_Flags.HasFlag(MovementFlags.Move))
			{
				Vector3 direction = (m_TargetLocation - m_Rigidbody.Position) * m_Direction;
				direction.Normalize();
				m_Velocity = direction * m_Entity.Speed;
			} else
			{
				m_Velocity = Vector3.Lerp(m_Rigidbody.LinearVelocity, Vector3.Zero, Frame.TimeStep * 5.0f);
				if (m_Velocity.Length() < Frame.TimeStep * 5.0f)
				{
					m_Velocity = Vector3.Zero;
				}
			}

			m_Rigidbody.LinearVelocity = m_Velocity;

			if(m_AfterPlayerThrowTimer)
			{
				m_AfterPlayerThrowTimer.Reset();

				m_Flags |= MovementFlags.Move;
				m_Target = EnemyTarget.TargetBall;
				m_Direction = 1.0f;
			}

			DebugRenderer.DrawCircle(m_Entity.Transform.Translation, Vector3.Right * Mathf.HalfPI, m_Entity.PickUpRadius, Color.Green);
		}

		protected override void OnEvent(EnemyEvent layerEvent)
		{
			switch (layerEvent)
			{
				case EnemyEvent.ChaseBall:
					m_AfterPlayerThrowTimer.Start();
					break;
				case EnemyEvent.RunFromPlayer:
					m_Target = EnemyTarget.TargetPlayer;
					m_Direction = -1.0f;

					m_Flags |= MovementFlags.Move;
					break;
				case EnemyEvent.BallGrabbed:
					m_Target = EnemyTarget.TargetPlayer;
					m_Direction = 1.0f;

					m_Flags &= ~MovementFlags.Move;
					break;
			}
		}

		public void OnHit(Entity entity)
		{
			if (entity.Name == "BouncyBall")
			{
				if (entity.GetComponent<RigidbodyComponent>().LinearVelocity.Length() > 6.0f)
				{
					m_Hit = true;
					m_RecoveryTimer.Reset();
				}

			}
		}
	}
}
