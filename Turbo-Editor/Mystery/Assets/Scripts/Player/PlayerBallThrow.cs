using System.Collections.Generic;
using Turbo;

namespace Mystery
{
	internal class PlayerBallThrow
	{
		Player m_Player;

		BouncyBall m_GrabbedBall;
		AABB m_GrabCollider;
		List<BouncyBall> m_BouncyBalls;

		internal List<BouncyBall> BouncyBalls => m_BouncyBalls;

		internal PlayerBallThrow(Player player)
		{
			m_Player = player;

			// Grabbing 
			m_BouncyBalls = new List<BouncyBall>
			{
				// First one is already spawned
				m_Player.FindEntityByName("BouncyBall").As<BouncyBall>()
			};

			m_GrabCollider = new AABB(Vector3.Zero, new Vector3(1.5f, 3.0f, 1.5f));
		}

		internal void OnUpdate()
		{
			bool isGrabButtonDown = Input.IsKeyDown(KeyCode.F);

			if (isGrabButtonDown)
			{
				// Update AABB collider
				m_GrabCollider.Center = m_Player.CurrentPosition + m_Player.Forward * m_Player.PickLength;

				// If player does not hold any balls => try to catch some
				if(m_GrabbedBall == null)
				{
					foreach (var bouncyBall in m_Player.BouncyBalls)
					{
						if (m_GrabCollider.Contains(bouncyBall.Transform.Translation))
						{
							if (bouncyBall.SetOwner(m_Player))
							{
								m_GrabbedBall = bouncyBall;
							}
						}
					}
				}
				else // Player holds a ball
				{
					var rb = m_GrabbedBall.GetComponent<RigidbodyComponent>();
					rb.Position = m_Player.CurrentPosition + m_Player.Forward * m_Player.PickLength + Vector3.Up * m_Player.PickHeight;
					rb.Rotation = m_Player.CurrentRotation;
					rb.LinearVelocity = Vector3.Zero;
					rb.AngularVelocity = Vector3.Zero;
				}
			} 
			else // Player doesnt hold grab button
			{
				// If player's hands are not empty => release ball
				if(m_GrabbedBall != null)
				{
					var rb = m_GrabbedBall.GetComponent<RigidbodyComponent>();

					if (m_GrabbedBall.Name == "BouncyBall")
						rb.AddForce(m_Player.Forward * m_Player.BallThrowPower, ForceMode.Impulse);

					m_GrabbedBall.Release(m_Player);
					m_GrabbedBall = null;
				}
			}

			// Debug
			if (isGrabButtonDown)
			{
				DebugRenderer.DrawBox(m_GrabCollider.Center, Vector3.Zero, m_GrabCollider.Size, Color.Green);
			}
		}
	}
}
