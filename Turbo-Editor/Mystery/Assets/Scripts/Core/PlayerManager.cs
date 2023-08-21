using Turbo;

namespace Mystery
{
	public class PlayerManager : Entity
	{
		public float LinearVelocityMagnifier;
		public float AngularVelocityMagnifier;

		internal PlayerInput m_Input;
		internal PlayerMovement m_Movement;

		private Entity m_TargetCursor;

		Entity m_Hat;

		Entity m_Ball;
		bool m_IsHolding = false;

		protected override void OnCreate()
		{
			m_Input = new PlayerInput();
			m_Movement = new PlayerMovement(this);
			m_Movement.RayCastHit += OnRayCastHit;

			m_Hat = FindEntityByName("Hat");
			m_Ball = FindEntityByName("Ball");
			m_TargetCursor = FindEntityByName("MOBA_Crosshair");

			Log.Info("Hello entity!");
		}

		protected override void OnUpdate()
		{
			m_Input.Update();
			m_Movement.Update();

			OnPickUpUpdate();
			
			// Hat
			var r = m_Hat.Transform.Rotation;
			r.XY -= Frame.TimeStep;
			m_Hat.Transform.Rotation = r;
		}

		void OnPickUpUpdate()
		{
			Vector3 dist = m_Ball.Transform.Translation - Transform.Translation;
			if (m_Input.IsPickUpButtonDown && dist.Length() < 5.0f)
			{
				m_IsHolding = true;

				Vector3 forward = new Quaternion(Transform.Rotation) * Vector3.Forward;
				forward.Y = 0.0f;
				forward.Normalize();

				var rb = m_Ball.GetComponent<RigidbodyComponent>();
				rb.Position = m_Movement.Rigidbody.Position + forward * 3.0f;
				rb.LinearVelocity = Vector3.Zero;
				rb.AngularVelocity = Vector3.Zero;
				rb.Rotation = m_Movement.Rigidbody.Rotation;
			}

			if (m_IsHolding && Input.IsKeyUp(KeyCode.F))
			{
				m_IsHolding = false;
				Vector3 forward = new Quaternion(Transform.Rotation) * Vector3.Forward;
				forward.Y = 0.0f;
				forward.Normalize();

				//var rb = m_Ball.GetComponent<RigidbodyComponent>();
				//rb.AddForce(forward * 50.0f, ForceMode.Impulse);
			}
		}

		void OnRayCastHit(Vector3 hitPosition)
		{
			var transform = m_TargetCursor.Transform;
			transform.Translation = hitPosition + Vector3.Up * 0.01f;
			transform.Rotation = Vector3.Right * Mathf.Radians(90.0f);
		}

	}
}
