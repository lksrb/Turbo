using Turbo;

namespace Mystery
{
	public class PlayerManager : Entity
	{
		public float LinearVelocityMagnifier;
		public float AngularVelocityMagnifier;

		internal PlayerInput m_Input;

		Entity m_ShootCube;
		Entity m_Camera;

		RigidbodyComponent m_Rigidbody;

		Vector2 m_MovementDirection = Vector2.Zero;
		Vector2 m_LastMousePosition = Vector2.Zero;
		Vector2 m_CurrentMousePos = Vector2.Zero;

		Vector3 m_Velocity = Vector3.Zero;
		Vector3 m_Destination = Vector3.Zero;
		bool m_Follows = false;

		Quaternion m_CurrentRotation;

		protected override void OnCreate()
		{
			m_Input = new PlayerInput();

			m_Camera = FindEntityByName("Camera");
			m_ShootCube = FindEntityByName("ShootCube");

			m_Rigidbody = GetComponent<RigidbodyComponent>();

			Log.Info("Hello entity!");

			m_LastMousePosition = Input.GetMousePosition();

			m_CurrentRotation = m_Rigidbody.Rotation;
		}

		bool pressed = false;

		protected override void OnUpdate()
		{
			m_Input.Update();
			m_CurrentMousePos = Input.GetMousePosition();

			OnMovementUpdate();
			OnPickUpUpdate();

			Entity cube = FindEntityByName("Cube1");

			var r = cube.Transform.Rotation;
			r.X -= Frame.TimeStep;
			cube.Transform.Rotation = r;
		}

		void OnMovementUpdate()
		{
			if (m_Input.IsSetDestinationButtonDown)
			{
				Vector3 worldPos = Camera.ScreenToWorldPosition(m_CurrentMousePos);
				worldPos.Normalize();

				Ray ray = new Ray(m_Camera.Transform.Translation, worldPos * 100.0f);
				if (Physics.CastRay(ray, RayTarget.Any, out RayCastResult result))
				{
					switch (result.HitEntity.Name)
					{
						case "Ground":
						case "Wall":
						case "PressurePlate":
							var transform = FindEntityByName("MOBACrosshair").Transform;
							transform.Translation = result.HitPosition + Vector3.Up * 0.01f;
							transform.Rotation = Vector3.Right * Mathf.Radians(90.0f);

							m_Destination = result.HitPosition;
							m_Follows = true;

							Vector3 direction = Transform.Translation - result.HitPosition;
							direction.Normalize();

							direction.Y = 0.0f;
							m_CurrentRotation = Quaternion.LookAt(direction, Vector3.Up);
							break;

					}
				}
			}
			else if (m_Input.IsFocusButtonDown)
			{
				Vector3 worldPos = Camera.ScreenToWorldPosition(m_CurrentMousePos);
				worldPos.Normalize();

				Ray ray = new Ray(m_Camera.Transform.Translation, worldPos * 50.0f);
				if (Physics.CastRay(ray, RayTarget.Closest, out RayCastResult result))
				{
					switch (result.HitEntity.Name)
					{
						case "TargetDummy":
							Vector3 direction1 = Transform.Translation - result.HitPosition;
							direction1.Normalize();

							direction1.Y = 0.0f;
							m_CurrentRotation = Quaternion.LookAt(direction1, Vector3.Up);

							m_Destination = Transform.Translation;
							m_Follows = false;
							m_Rigidbody.LinearVelocity = Vector3.Zero;
							break;
					}
				}
			}


			m_Rigidbody.Rotation = Quaternion.Slerp(m_Rigidbody.Rotation, m_CurrentRotation, Frame.TimeStep * AngularVelocityMagnifier);

			if (m_Follows)
			{
				m_Follows = (m_Destination - Transform.Translation).Length() > Frame.TimeStep;

				Vector3 distance = m_Destination - Transform.Translation;
				distance.Normalize();

				m_Velocity = distance * LinearVelocityMagnifier;
				m_Velocity.Y = m_Rigidbody.LinearVelocity.Y;
				m_Rigidbody.LinearVelocity = m_Velocity;
			}

		}

		void OnPickUpUpdate()
		{
			Vector3 dist = m_ShootCube.Transform.Translation - Transform.Translation;
			if (m_Input.IsPickUpButtonDown && dist.Length() < 5.0f)
			{
				pressed = true;

				Vector3 forward = new Quaternion(Transform.Rotation) * Vector3.Forward;
				forward.Y = 0.0f;
				forward.Normalize();

				var rb = m_ShootCube.GetComponent<RigidbodyComponent>();
				rb.Position = m_Rigidbody.Position + forward * 3.0f;
				rb.Rotation = m_Rigidbody.Rotation;
			}

			if (pressed && Input.IsKeyUp(KeyCode.F))
			{
				pressed = false;
				Vector3 forward = new Quaternion(Transform.Rotation) * Vector3.Forward;
				forward.Y = 0.0f;
				forward.Normalize();

				var rb = m_ShootCube.GetComponent<RigidbodyComponent>();
				rb.AddForce(forward * 50.0f, ForceMode.Impulse);
			}
		}


	}
}
