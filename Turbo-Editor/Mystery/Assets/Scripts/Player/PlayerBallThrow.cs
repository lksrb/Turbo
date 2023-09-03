using Turbo;

namespace Mystery
{
	internal class PlayerBallThrow : Layer<Player, PlayerEvent>
	{
		PlayerInput m_Input;

		Entity m_GrabbedItem;
		BouncyBall m_BouncyBall;

		RigidbodyComponent m_Rigidbody;
		AABB m_GrabCollider;

		protected override void OnAttach()
		{
			m_Input = Get<PlayerInput>();
			m_Rigidbody = m_Entity.GetComponent<RigidbodyComponent>();

			m_BouncyBall = m_Entity.FindEntityByName("BouncyBall").As<BouncyBall>();

			m_GrabCollider = new AABB(Vector3.Zero, new Vector3(1.5f, 3.0f, 1.5f));
		}

		protected override void OnUpdate()
		{
			Vector3 forward = new Quaternion(m_Entity.Transform.Rotation) * Vector3.Forward;
			forward.Y = 0.0f;
			forward.Normalize();

			m_GrabCollider.Center = m_Entity.Transform.Translation + forward * m_Entity.PickLength;
			
			if (m_GrabbedItem == null && m_Input.IsPickUpButtonDown)
			{
				if (m_GrabCollider.Contains(m_BouncyBall.Transform.Translation))
				{
					if (m_BouncyBall.SetHolder(m_Entity))
					{
						m_GrabbedItem = m_BouncyBall;
						Emit(PlayerEvent.BallGrabbed);
					}
				}
			}

			if (m_GrabbedItem != null)
			{
				var rb = m_GrabbedItem.GetComponent<RigidbodyComponent>();
				rb.Position = m_Rigidbody.Position + forward * m_Entity.PickLength + Vector3.Up * m_Entity.PickHeight;
				rb.Rotation = m_Rigidbody.Rotation;
				rb.LinearVelocity = Vector3.Zero;
				rb.AngularVelocity = Vector3.Zero;

				if (Input.IsKeyUp(KeyCode.F))
				{
					if (m_GrabbedItem.Name == "BouncyBall")
						rb.AddForce(forward * m_Entity.BallThrowPower, ForceMode.Impulse);

					Emit(PlayerEvent.BallThrew);
					m_BouncyBall.ReleaseFromHolder(m_Entity);
					m_GrabbedItem = null;
				}
			}

			// Debug
			if (m_Input.IsPickUpButtonDown)
			{
				DebugRenderer.DrawBox(m_GrabCollider.Center, Vector3.Zero, m_GrabCollider.Size, Color.Green);
			}
		}

		protected override void OnEvent(PlayerEvent playerEvent)
		{
		}
	}
}
