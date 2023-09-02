using Turbo;

namespace Mystery
{
	internal class PlayerBallPick : PlayerModule
	{
		PlayerInput m_Input;

		Entity m_PickedItem;
		BouncyBall m_BouncyBall;

		RigidbodyComponent m_Rigidbody;
		AABB m_GrabCollider;

		protected override void OnAttach()
		{
			m_Input = Get<PlayerInput>();
			m_Rigidbody = m_Player.GetComponent<RigidbodyComponent>();

			m_BouncyBall = m_Player.FindEntityByName("BouncyBall").As<BouncyBall>();
		}

		protected override void OnUpdate()
		{
			Vector3 forward = new Quaternion(m_Player.Transform.Rotation) * Vector3.Forward;
			forward.Y = 0.0f;
			forward.Normalize();

			m_GrabCollider = new AABB(m_Player.Transform.Translation + forward * m_Player.PickLength, new Vector3(1.0f, 2.0f, 1.0f));

			if (m_PickedItem == null && m_Input.IsPickUpButtonDown)
			{
				if (m_GrabCollider.Contains(m_BouncyBall.Transform.Translation))
				{
					if (m_BouncyBall.SetHolder(m_Player))
					{
						m_PickedItem = m_BouncyBall;
						Emit(PlayerEvent.OnBallPicked);
					}
				}
			}

			if (m_PickedItem != null)
			{
				var rb = m_PickedItem.GetComponent<RigidbodyComponent>();
				rb.Position = m_Rigidbody.Position + forward * m_Player.PickLength + Vector3.Up * m_Player.PickHeight;
				rb.Rotation = m_Rigidbody.Rotation;
				rb.LinearVelocity = Vector3.Zero;
				rb.AngularVelocity = Vector3.Zero;

				if (Input.IsKeyUp(KeyCode.F))
				{
					if (m_PickedItem.Name == "BouncyBall")
						rb.AddForce(forward * m_Player.BallThrowPower, ForceMode.Impulse);

					Emit(PlayerEvent.OnBallThrew);
					m_BouncyBall.ReleaseFromHolder(m_Player);
					m_PickedItem = null;
				}
			}

			// Debug
			if(m_Input.IsPickUpButtonDown)
			{
				DebugRenderer.DrawBox(m_GrabCollider.Center, m_Player.Transform.Rotation, m_GrabCollider.Size, Color.Green);
			}
		}

		protected override void OnEvent(PlayerEvent playerEvent)
		{
		}
	}
}
