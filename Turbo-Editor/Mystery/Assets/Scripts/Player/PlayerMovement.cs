using System;
using Turbo;

namespace Mystery
{
	enum MovementFlags : uint
	{
		Move = 1 << 0,
		Rotate = 1 << 1,
		All = Move | Rotate
	}

	internal class PlayerMovement : PlayerModule
	{
		PlayerInput m_Input;

		RigidbodyComponent m_Rigidbody;

		TransformComponent m_CameraTransform;
		Vector2 m_CurrentMousePosition;

		// Flags to restrict movement or allow some of it
		MovementFlags m_MovementFlags = MovementFlags.All;

		// Callback for target cursor
		internal Action<Vector3> m_OnChangeTargetLocation;

		// Track linear velocity
		Vector3 m_LinearVelocity = Vector3.Zero;

		// Targets
		Vector3 m_TargetLocation;
		Quaternion m_TargetRotation;

		protected override void OnAttach()
		{
			m_Input = Get<PlayerInput>();

			m_CameraTransform = m_Player.FindEntityByName("Camera").Transform;
			m_Rigidbody = m_Player.GetComponent<RigidbodyComponent>();
			m_CurrentMousePosition = Input.GetMousePosition();

			m_TargetLocation = m_Rigidbody.Position;
			m_TargetRotation = m_Rigidbody.Rotation;
		}

		protected override void OnUpdate()
		{
			m_CurrentMousePosition = Input.GetMousePosition();

			if (m_Input.IsSetDestinationButtonDown)
			{
				// This is more of a direction than a world mouse position since we cannot directly know the Z-axis
				Vector3 mouseWorldPosition = Camera.ScreenToWorldPosition(m_CurrentMousePosition);
				mouseWorldPosition.Normalize();

				// Create a ray that origins from camera's position and casts to world mouse position
				Ray ray = new Ray(m_CameraTransform.Translation, mouseWorldPosition * 100.0f);

				// Cast ray that only receives furthest hit - this is because we want to only hit Ground entity since its the furthest
				if (Physics.CastRay(ray, RayTarget.Furthest, out RayCastResult result))
				{
					if (m_MovementFlags.HasFlag(MovementFlags.Move))
					{
						// Assign target position
						m_TargetLocation = result.HitPosition;

						// Invoke event callback for target cursor
						m_OnChangeTargetLocation?.Invoke(m_TargetLocation);
					}

					// Calculate direction
					Vector3 direction = Vector3.Normalize(m_Rigidbody.Position - result.HitPosition);

					// Assign and calculate forward target rotation
					m_TargetRotation = Quaternion.LookAt(new Vector3(direction.X, 0.0f, direction.Z), Vector3.Up);
				}
			}

			// Take care of movement to the target location
			MoveAndRotateTowardsTarget();
		}

		private void MoveAndRotateTowardsTarget()
		{
			// Change velocity according to direction towards target

			if (m_MovementFlags.HasFlag(MovementFlags.Move))
			{
				Vector3 distance = m_TargetLocation - m_Rigidbody.Position;
				if (distance.Length() > Frame.TimeStep)
				{
					m_LinearVelocity = Vector3.Normalize(distance) * m_Player.LinearVelocityMagnifier;
				}
				else
				{
					m_LinearVelocity = Vector3.Zero;
				}
			} 
			else // Cannot move
			{
				m_LinearVelocity = Vector3.Lerp(m_LinearVelocity, Vector3.Zero, Frame.TimeStep * 5.0f);
				m_TargetLocation = m_Rigidbody.Position;
				if (m_LinearVelocity.Length() < Frame.TimeStep * 5.0f)
				{
					m_LinearVelocity = Vector3.Zero;
				}
			}

			m_Rigidbody.LinearVelocity = m_LinearVelocity;

			if (m_MovementFlags.HasFlag(MovementFlags.Rotate))
			{
				// Smoothly change rotation according to direction towards target
				m_Rigidbody.Rotation = Quaternion.Slerp(m_Rigidbody.Rotation, m_TargetRotation, Frame.TimeStep * m_Player.AngularVelocityMagnifier);
			}
		}

		protected override void OnEvent(PlayerEvent playerEvent)
		{
			switch (playerEvent)
			{
				case PlayerEvent.OnBallPicked:
					m_MovementFlags &= ~MovementFlags.Move;
					break;
				case PlayerEvent.OnBallThrew:
					m_MovementFlags |= MovementFlags.Move;
					break;
			}
		}
	}
}
