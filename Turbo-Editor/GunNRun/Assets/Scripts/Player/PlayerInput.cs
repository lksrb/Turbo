using Turbo;

namespace GunNRun
{
	internal class PlayerInput
	{

		// ---- Moving ----
		private readonly KeyCode m_MoveUpKeyCode = KeyCode.W;
		private readonly KeyCode m_MoveDownKeyCode = KeyCode.S;
		private readonly KeyCode m_MoveLeftKeyCode = KeyCode.A;
		private readonly KeyCode m_MoveRightKeyCode = KeyCode.D;
		internal bool IsLeftKeyPressed { get; private set; }
		internal bool IsRightKeyPressed { get; private set; }
		internal bool IsUpKeyPressed { get; private set; }
		internal bool IsDownKeyPressed { get; private set; }

		// ---- Shooting ----
		private readonly KeyCode m_ShootKeyCode = KeyCode.L;
		private bool m_IsShootKeyReleased = true;
		private bool m_WasShootKeyReleasedLastFrame = true;
		internal bool IsShootKeyPressed { get; private set; }
		internal bool IsShootKeyPressedOneTime => m_WasShootKeyReleasedLastFrame && IsShootKeyPressed;

		internal void OnUpdate()
		{
			// ---- Moving ----
			IsLeftKeyPressed = Input.IsKeyPressed(m_MoveLeftKeyCode);
			IsRightKeyPressed = Input.IsKeyPressed(m_MoveRightKeyCode);
			IsUpKeyPressed = Input.IsKeyPressed(m_MoveUpKeyCode);
			IsDownKeyPressed = Input.IsKeyPressed(m_MoveDownKeyCode);

			// ---- Shooting ----
			m_WasShootKeyReleasedLastFrame = m_IsShootKeyReleased;
			IsShootKeyPressed = Input.IsKeyPressed(m_ShootKeyCode);
			m_IsShootKeyReleased = Input.IsKeyReleased(m_ShootKeyCode);
		}
	}
}
