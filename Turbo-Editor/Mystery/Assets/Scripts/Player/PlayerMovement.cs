using System;
using Turbo;

namespace Mystery
{
	internal class PlayerMovement
	{
		Player m_Player;

		internal Vector3 Translation;
		internal Vector3 Velocity;
		internal RigidbodyComponent Rigidbody;
		internal Action<Vector3> RayCastHit;

		Entity m_Camera;
		Vector2 m_CurrentMousePos = Vector2.Zero;

		Vector3 m_TargetLocation;
		Quaternion m_TargetRotation;

		internal PlayerMovement(Player manager)
		{
			Rigidbody = manager.GetComponent<RigidbodyComponent>();

			m_Player = manager;
			m_Camera = manager.FindEntityByName("Camera");
			m_TargetLocation = Rigidbody.Position;
			m_TargetRotation = Rigidbody.Rotation;
		}

		internal void Update()
		{
			m_CurrentMousePos = Input.GetMousePosition();
			Translation = m_Player.Transform.Translation;

			if (m_Player.m_Input.IsSetDestinationButtonDown)
			{
				Vector3 worldPos = Camera.ScreenToWorldPosition(m_CurrentMousePos);
				worldPos.Normalize();

				Ray ray = new Ray(m_Camera.Transform.Translation, worldPos * 100.0f);
				if (Physics.CastRay(ray, RayTarget.Furthest, out RayCastResult result))
				{
					switch (result.HitEntity.Name)
					{
						case "Ground":
						case "Wall":
						case "PressurePlate":
							m_TargetLocation = result.HitPosition;

							Vector3 direction = Translation - m_TargetLocation;
							direction.Y = 0.0f;
							direction.Normalize();

							m_TargetRotation = Quaternion.LookAt(direction, Vector3.Up);

							RayCastHit?.Invoke(result.HitPosition);
							break;

					}
				}
			}
			else if (m_Player.m_Input.IsFocusButtonDown)
			{
				Vector3 worldPos = Camera.ScreenToWorldPosition(m_CurrentMousePos);
				worldPos.Normalize();

				Ray ray = new Ray(m_Camera.Transform.Translation, worldPos * 50.0f);
				if (Physics.CastRay(ray, RayTarget.Closest, out RayCastResult result))
				{
					switch (result.HitEntity.Name)
					{
						case "TargetDummy":
							Vector3 direction = Translation - result.HitPosition;
							direction.Normalize();

							direction.Y = 0.0f;
							m_TargetRotation = Quaternion.LookAt(direction, Vector3.Up);

							m_TargetLocation = Translation;
							Rigidbody.LinearVelocity = Vector3.Zero;
							break;
					}
				}
			}

			Rigidbody.Rotation = Quaternion.Slerp(Rigidbody.Rotation, m_TargetRotation, Frame.TimeStep * m_Player.AngularVelocityMagnifier);

			if ((m_TargetLocation - Translation).Length() > Frame.TimeStep)
			{
				Vector3 distance = m_TargetLocation - Translation;
				distance.Normalize();

				Velocity = distance * m_Player.LinearVelocityMagnifier;
				Velocity.Y = Rigidbody.LinearVelocity.Y;
				Rigidbody.LinearVelocity = Velocity;
			}

		}
	}
}
