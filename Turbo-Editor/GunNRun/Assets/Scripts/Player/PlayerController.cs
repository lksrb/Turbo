using Turbo;

namespace GunNRun
{
	internal enum PlayerDirection : uint
	{
		Left = 0,
		Right,
		Up,
		Down
	}

	internal class PlayerController
	{
		private PlayerManager m_PlayerManager;
		private PlayerInput m_PlayerInput;

		private Rigidbody2DComponent m_RigidBody2D;
		private TransformComponent m_Transform;

		private bool m_IsGrounded = false;

		internal PlayerDirection Direction { get; private set; } = PlayerDirection.Right;
		internal bool IsMovingSideways { get; private set; } = false;
		internal bool IsInAir { get; private set; } = false;

		internal void Init(PlayerManager playerManager)
		{
			m_PlayerManager = playerManager;
			m_PlayerInput = m_PlayerManager.m_PlayerInput;

			m_RigidBody2D = m_PlayerManager.GetComponent<Rigidbody2DComponent>();
			m_Transform = m_PlayerManager.Transform;

			// Set callbacks
			m_PlayerManager.OnCollisionBegin2D += OnCollisionBegin;
			m_PlayerManager.OnCollisionEnd2D += OnCollisionEnd;
		}

		internal void OnUpdate(float ts)
		{
			//m_IsGrounded = Mathf.Abs(m_RigidBody2D.Velocity.y) < ts;
			IsMovingSideways = Mathf.Abs(m_RigidBody2D.Velocity.X) > ts;

			if (m_PlayerInput.IsJumpKeyPressedOneTime && m_IsGrounded)
			{
				m_IsGrounded = false;
				m_RigidBody2D.ApplyForceToCenter(Vector2.Up * m_PlayerManager.m_JumpPower);
			}

			IsInAir = !m_IsGrounded;

			// Movement
			Vector2 velocity = new Vector2(0, m_RigidBody2D.Velocity.Y);

			if (m_PlayerInput.IsMoveRightKeyPressed)
			{
				Direction = PlayerDirection.Right;

				// Rotate it to right
				m_Transform.Scale = Mathf.Abs(m_Transform.Scale);

				velocity.X = m_PlayerManager.m_Speed;
			}
			if (m_PlayerInput.IsMoveLeftKeyPressed)
			{
				Direction = PlayerDirection.Left;

				Vector3 rotatedScale = new Vector3(-Mathf.Abs(m_Transform.Scale.X), m_Transform.Scale.Y, m_Transform.Scale.Z);
				m_Transform.Scale = rotatedScale;

				velocity.X = (-1) * m_PlayerManager.m_Speed;
			}

			m_RigidBody2D.Velocity = velocity;
		}

		private void OnCollisionBegin(Entity other)
		{
			// Simple filtering
			if (!FilterCollision(other))
				return;

			if (m_RigidBody2D.Velocity.Y < 0.0f)
			{
				m_IsGrounded = true;
			}

			Log.Info($"{m_RigidBody2D.Velocity}");
		}

		private void OnCollisionEnd(Entity other)
		{
			if (!FilterCollision(other))
				return;

			if (m_RigidBody2D.Velocity.X == 0.0f)
			{
				m_IsGrounded = false;
			}
		}
		
		private bool FilterCollision(Entity entity)
		{
			return entity.Name.Contains("Hitbox-Horizontal");
		}

	}
}
