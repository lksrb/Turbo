using System;
using Turbo;

namespace Mystery
{
	internal class PlayerMovement
	{
		Player m_Player;

		TransformComponent m_CameraTransform;
		Vector2 m_CurrentMousePosition;

		// Visual target cursor
		Entity m_TargetCrosshair;

		// Targets
		Vector3 m_TargetLocation;
		Quaternion m_TargetRotation;

		internal PlayerMovement(Player player)
		{
			m_Player = player;

			m_CameraTransform = m_Player.FindEntityByName("Camera").Transform;
			m_TargetCrosshair = m_Player.FindEntityByName("TargetCursor");

			m_TargetLocation = m_Player.CurrentPosition;
			m_TargetRotation = m_Player.CurrentRotation;

			Log.Info("Player is moving!");
		}

		internal void OnUpdate()
		{
			m_CurrentMousePosition = Input.GetMousePosition();

			if (m_Player.Input.IsSetDestinationButtonDown)
			{
				// This is more of a direction than a world mouse position since we cannot directly know the Z-axis
				Vector3 mouseWorldPosition = Camera.ScreenToWorldPosition(m_CurrentMousePosition);
				mouseWorldPosition.Normalize();

				// Create a ray that origins from camera's position and casts to world mouse position
				// Query all hits - this is an expensive operation - maybe we shouldnt create new entity classes for each hit
				if (Physics.CastRayAll(m_CameraTransform.Translation, mouseWorldPosition, Mathf.Infinity, out CastRayAllResult allResult))
				{
					foreach (CastRayResult result in allResult)
					{
						if (result.HitEntity.Name == "TargetCursorGround")
						{
							// Assign target position
							m_TargetLocation = result.HitPosition;

							// Calculate direction
							Vector3 direction = Vector3.Normalize(result.HitPosition - m_Player.CurrentPosition);

							// Assign and calculate forward target rotation
							m_TargetRotation = Quaternion.LookAt(new Vector3(direction.X, 0.0f, direction.Z), Vector3.Up);
						}
					}
				}
			}

			// Take care of movement to the target location
			MoveAndRotateTowardsTarget();

			// Move target cursor
			var transform = m_TargetCrosshair.Transform;
			transform.Translation = m_TargetLocation + Vector3.Up * 0.03f;
			transform.Rotation = Vector3.Right * Mathf.Radians(90.0f);

			// Avoid jiggering
			if (m_Player.LinearVelocity.XZ.Length() < Frame.TimeStep * m_Player.LinearVelocityMagnifier)
			{
				m_Player.LinearVelocity = Vector3.Zero;
				m_Player.CurrentRotation = m_TargetRotation;
			}

		}

		private void MoveAndRotateTowardsTarget()
		{
			Vector3 distance = m_TargetLocation - m_Player.CurrentPosition;

			if (distance.Length() > 0.0f)
			{
				// Move player to target destination - velocity is based on the distance
				m_Player.LinearVelocity = Vector3.Normalize(distance) * m_Player.LinearVelocityMagnifier;
			}

			// Smoothly change rotation according to direction towards target
			m_Player.CurrentRotation = Quaternion.Slerp(m_Player.CurrentRotation, m_TargetRotation, Frame.TimeStep * m_Player.AngularVelocityMagnifier);
		}
	}
}
