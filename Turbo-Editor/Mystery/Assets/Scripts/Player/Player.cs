using System.Collections.Generic;
using Turbo;

namespace Mystery
{
	// State machine
	public class Player : Entity
	{
		public readonly float LinearVelocityMagnifier;
		public readonly float AngularVelocityMagnifier;
		public readonly float PickLength;
		public readonly float PickHeight;
		public readonly float PickUpRadius;
		public readonly float LockMovementDeltaMagnifier;
		public readonly float BallThrowPower;

		// Components
		private PlayerInput m_Input;
		private PlayerMovement m_Movement;
		private PlayerBallThrow m_BallThrow;

		private Entity m_Hat;

		private Vector3 m_LinearVelocity;
		private Vector3 m_CurrentPosition;
		private Quaternion m_CurrentRotation;
		private Vector3 m_ForwardDirection;
		private RigidbodyComponent m_Rigidbody;

		// Getters & Setters
		internal Quaternion CurrentRotation { get => m_CurrentRotation; set { m_CurrentRotation = value; } }
		internal Vector3 CurrentPosition => m_CurrentPosition;
		internal Vector3 LinearVelocity { get => m_LinearVelocity; set { m_LinearVelocity = value; } }
		internal List<BouncyBall> BouncyBalls => m_BallThrow.BouncyBalls;
		internal PlayerInput Input => m_Input;
		internal Vector3 Forward => m_ForwardDirection;

		protected override void OnCreate()
		{
			m_Hat = FindEntityByName("Hat");
			m_Rigidbody = GetComponent<RigidbodyComponent>();

			// Movement
			m_CurrentPosition = m_Rigidbody.Position;
			m_CurrentRotation = m_Rigidbody.Rotation;
			m_LinearVelocity = Vector3.Zero;
			m_ForwardDirection = CalculateForwardDirection();

			// Components
			m_Input = new PlayerInput();
			m_Movement = new PlayerMovement(this);
			m_BallThrow = new PlayerBallThrow(this);

			Log.Info("Hello from player!");
		}

		protected override void OnUpdate()
		{
			m_ForwardDirection = CalculateForwardDirection();
			m_CurrentPosition = m_Rigidbody.Position;

			// Update components
			m_Input.OnUpdate();
			m_Movement.OnUpdate();
			m_BallThrow.OnUpdate();

			// Update body
			m_Rigidbody.Rotation = m_CurrentRotation;
			m_Rigidbody.LinearVelocity = m_LinearVelocity;

			// Hat
			var r = m_Hat.Transform.Rotation;
			r.XY -= Frame.TimeStep;
			m_Hat.Transform.Rotation = r;
		}

		private Vector3 CalculateForwardDirection()
		{
			Vector3 result = m_CurrentRotation * Vector3.Forward;
			result.Y = 0.0f;
			result.Normalize();
			return result;
		}
	}
}
