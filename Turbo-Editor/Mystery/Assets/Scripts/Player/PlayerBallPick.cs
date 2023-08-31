using Turbo;

namespace Mystery
{
	internal class PlayerBallPick : PlayerLayer
	{
		PlayerInput m_Input;
		PlayerMovement m_Movement;

		Entity m_PickedItem;
		BouncyBall m_BouncyBall;

		protected override void OnAttach()
		{
			m_Input = Get<PlayerInput>();
			m_Movement = Get<PlayerMovement>();

			m_BouncyBall = m_Player.FindEntityByName("BouncyBall").As<BouncyBall>();
		}

		protected override void OnUpdate()
		{
			Vector3 forward = new Quaternion(m_Player.Transform.Rotation) * Vector3.Forward;
			forward.Y = 0.0f;
			forward.Normalize();

			if (m_PickedItem == null && m_Input.IsPickUpButtonDown)
			{
				if (CollisionTest.IsInCircle(m_BouncyBall.Transform.Translation, m_Player.Transform.Translation, m_Player.PickUpRadius))
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
				rb.Position = m_Movement.Rigidbody.Position + forward * m_Player.PickLength + Vector3.Up * m_Player.PickHeight;
				rb.Rotation = m_Movement.Rigidbody.Rotation;
				rb.LinearVelocity = Vector3.Zero;
				rb.AngularVelocity = Vector3.Zero;

				if (Input.IsKeyUp(KeyCode.F))
				{
					if (m_PickedItem.Name == "BouncyBall")
						rb.AddForce(forward * 50.0f, ForceMode.Impulse);

					Emit(PlayerEvent.OnBallThrew);
					m_BouncyBall.ReleaseFromHolder(m_Player);
					m_PickedItem = null;
				}
			}

			DebugRenderer.DrawCircle(m_Player.Transform.Translation, Vector3.Right * Mathf.HalfPI, m_Player.PickUpRadius, Color.Green);
		}

		protected override void OnEvent(PlayerEvent playerEvent)
		{
		}
	}
}
