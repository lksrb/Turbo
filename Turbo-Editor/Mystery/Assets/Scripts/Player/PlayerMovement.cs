﻿using System;
using Turbo;

namespace Mystery
{
	internal class PlayerMovement : Layer<Player, PlayerEvent>
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

			m_CameraTransform = m_Entity.FindEntityByName("Camera").Transform;
			m_Rigidbody = m_Entity.GetComponent<RigidbodyComponent>();
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
				// Query all hits - this is an expensive operation - maybe we shouldnt create new entity classes for each hit
				if (Physics.CastRayAll(m_CameraTransform.Translation, mouseWorldPosition, Mathf.Infinity, out CastRayAllResult allResult))
				{
					foreach(CastRayResult result in allResult)
					{
						if(result.HitEntity.Name == "TargetCursorGround")
						{
							// Assign target position
							m_TargetLocation = result.HitPosition;

							// Invoke event callback for target cursor
							m_OnChangeTargetLocation?.Invoke(m_TargetLocation);

							// Calculate direction
							Vector3 direction = Vector3.Normalize(result.HitPosition - m_Rigidbody.Position);

							// Assign and calculate forward target rotation
							m_TargetRotation = Quaternion.LookAt(new Vector3(direction.X, 0.0f, direction.Z), Vector3.Up);
						}
					}
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
					m_LinearVelocity = Vector3.Normalize(distance) * m_Entity.LinearVelocityMagnifier;
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
				m_Rigidbody.Rotation = Quaternion.Slerp(m_Rigidbody.Rotation, m_TargetRotation, Frame.TimeStep * m_Entity.AngularVelocityMagnifier);
			}
		}

		protected override void OnEvent(PlayerEvent playerEvent)
		{
			switch (playerEvent)
			{
				case PlayerEvent.BallGrabbed:
					//m_MovementFlags &= ~MovementFlags.Move;
					break;
				case PlayerEvent.BallThrew:
					//m_MovementFlags |= MovementFlags.Move;
					break;
			}
		}
	}
}
