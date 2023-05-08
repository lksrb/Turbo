using Turbo;

namespace GunNRun
{
	internal class PlayerInput
	{
		private PlayerManager m_PlayerManager;

		private readonly KeyCode m_JumpKeyCode = KeyCode.Space;
		private readonly KeyCode m_ShootKeyCode = KeyCode.L;
		private readonly KeyCode m_MoveLeftKeyCode = KeyCode.A;
		private readonly KeyCode m_MoveRightKeyCode = KeyCode.D;
		private readonly KeyCode m_CrouchKeyCode = KeyCode.LeftControl;

		// ---- Jumping ----
		private bool m_IsJumpKeyReleased = true;
		private bool m_WasJumpKeyReleasedLastFrame = true;

		internal bool IsJumpKeyPressed { get; private set; }
		internal bool IsJumpKeyPressedOneTime => m_WasJumpKeyReleasedLastFrame && IsJumpKeyPressed;

		// ---- Crouchning ----
		internal bool IsCrouchKeyHold { get; private set; } = false;

		// ---- Moving ----
		internal bool IsMoveLeftKeyPressed { get; private set; }
		internal bool IsMoveRightKeyPressed { get; private set; }

		// ---- Shooting ----
		private bool m_IsShootKeyReleased = true;
		private bool m_WasShootKeyReleasedLastFrame = true;

		internal bool IsShootKeyPressed { get; private set; }
		internal bool IsShootKeyPressedOneTime => m_WasShootKeyReleasedLastFrame && IsShootKeyPressed;

		internal void Init(PlayerManager playerManager)
		{
			m_PlayerManager = playerManager;
		}

		internal void OnUpdate(float ts)
		{
			// ---- Jumping ----
			m_WasJumpKeyReleasedLastFrame = m_IsJumpKeyReleased;
			IsJumpKeyPressed = Input.IsKeyPressed(m_JumpKeyCode);
			m_IsJumpKeyReleased = Input.IsKeyReleased(m_JumpKeyCode);

			// ---- Crouching -----
			IsCrouchKeyHold = Input.IsKeyPressed(m_CrouchKeyCode);

			// ---- Moving ----
			IsMoveLeftKeyPressed = Input.IsKeyPressed(m_MoveLeftKeyCode);
			IsMoveRightKeyPressed = Input.IsKeyPressed(m_MoveRightKeyCode);

			// ---- Shooting ----
			m_WasShootKeyReleasedLastFrame = m_IsShootKeyReleased;
			IsShootKeyPressed = Input.IsKeyPressed(m_ShootKeyCode);
			m_IsShootKeyReleased = Input.IsKeyReleased(m_ShootKeyCode);
		}
	}
}
