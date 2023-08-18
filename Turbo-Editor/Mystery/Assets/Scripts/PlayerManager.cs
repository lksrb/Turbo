using System;
using System.Runtime.InteropServices;
using System.Threading;
using Turbo;

namespace Sandbox
{
	public class PlayerManager : Entity
	{
		public float m_Speed;

		Entity m_Camera;

		RigidbodyComponent m_Rigidbody;

		Vector2 m_MovementDirection = Vector2.Zero;
		Vector2 m_LastMousePosition = Vector2.Zero;

		Vector3 m_Velocity = Vector3.Zero;
		Vector3 m_Destination = Vector3.Zero;
		bool m_Follows = false;

		Quaternion m_CurrentRotation;

		protected override void OnCreate()
		{
			//Input.SetCursorMode(CursorMode.Locked);

			m_Camera = FindEntityByName("Camera");

			m_Rigidbody = GetComponent<RigidbodyComponent>();

			Log.Info("Hello entity!");

			m_LastMousePosition = Input.GetMousePosition();

			m_CurrentRotation = m_Rigidbody.Rotation;
		}

		private bool m_IsShootKeyReleased = true;
		private bool m_WasShootMouseButtonPressed = false;
		private bool m_IsShootMouseButtonPressed = false;
		internal bool IsShootMouseButtonPressed => m_WasShootMouseButtonPressed && m_IsShootMouseButtonPressed;

		protected override void OnUpdate()
		{
			// Shooting
			m_WasShootMouseButtonPressed = m_IsShootKeyReleased;
			m_IsShootMouseButtonPressed = Input.IsMouseButtonDown(MouseCode.ButtonLeft);
			m_IsShootKeyReleased = Input.IsMouseButtonUp(MouseCode.ButtonLeft);

			Vector2 mousePos = Input.GetMousePosition();

			if (IsShootMouseButtonPressed)
			{
				Vector3 worldPos = Camera.ScreenToWorldPosition(mousePos);
				worldPos.Normalize();

				Ray ray = new Ray(m_Camera.Transform.Translation, worldPos * 100.0f);
				if (Physics.CastRay(ray, RayTarget.Closest, out RayCastResult result))
				{
					if (result.HitEntity.Name == "Ground")
					{
						var transform = FindEntityByName("MOBACrosshair").Transform;
						transform.Translation = result.HitPosition + Vector3.Up * 0.01f;
						transform.Rotation = Vector3.Right * Mathf.Radians(90.0f);

						m_Destination = result.HitPosition;
						m_Follows = true;

						Vector3 direction = Transform.Translation - m_Destination;
						direction.Normalize();

						direction.Y = 0.0f;
						m_CurrentRotation = Quaternion.LookAt(direction, Vector3.Up);
					}
					else if (result.HitEntity.Name == "PickupCube")
					{
						var transform = FindEntityByName("MOBACrosshair").Transform;
						transform.Translation = result.HitPosition + Vector3.Up * 0.01f;
						transform.Rotation = Vector3.Right * Mathf.Radians(90.0f);

						Vector3 direction = Transform.Translation - m_Destination;
						direction.Y = 0.0f;
						direction.Normalize();

						m_Destination = result.HitPosition;
						m_Follows = true;

						m_CurrentRotation = Quaternion.LookAt(direction, Vector3.Up);
					}
				}
			}

			m_Rigidbody.Rotation = Quaternion.Slerp(m_Rigidbody.Rotation, m_CurrentRotation, Frame.TimeStep * 8.0f);

			Entity cube = FindEntityByName("Cube1");

			var r = cube.Transform.Rotation;
			r.X -= Frame.TimeStep;
			cube.Transform.Rotation = r;

			//Quaternion q = Quaternion.LookAt(new Vector3(0,1,-1), Vector3.Up);

			//cube.Transform.Rotation += q * Vector3.Left * Frame.TimeStep;
			//cube.Transform.Rotation = (q * Vector3.Left) * Frame.TimeStep;

			if (m_Follows)
			{
				m_Follows = (m_Destination - Transform.Translation).Length() > Frame.FixedTimeStep;

				Vector3 distance = m_Destination - Transform.Translation;
				distance.Normalize();

				m_Velocity = distance * m_Speed;
				m_Velocity.Y = m_Rigidbody.LinearVelocity.Y;

				m_Rigidbody.LinearVelocity = m_Velocity;
			}

		}
	}
}
