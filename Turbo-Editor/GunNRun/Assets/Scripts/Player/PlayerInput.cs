using Turbo;

namespace GunNRun
{
	internal class PlayerInput
	{
		private readonly KeyCode m_MoveUpKeyCode = KeyCode.W;
		private readonly KeyCode m_MoveDownKeyCode = KeyCode.S;
		private readonly KeyCode m_MoveLeftKeyCode = KeyCode.A;
		private readonly KeyCode m_MoveRightKeyCode = KeyCode.D;
		private readonly MouseCode m_ShootMouseCode = MouseCode.ButtonLeft;

		// ---- Moving ----
		internal bool IsLeftKeyPressed { get; private set; }
		internal bool IsRightKeyPressed { get; private set; }
		internal bool IsUpKeyPressed { get; private set; }
		internal bool IsDownKeyPressed { get; private set; }

		// ---- Shooting ----
		private bool m_IsShootKeyReleased = true;
		private bool m_WasShootMouseButtonPressed = false;
		private bool m_IsShootMouseButtonPressed = false;
		internal bool IsShootMouseButtonPressed => m_WasShootMouseButtonPressed && m_IsShootMouseButtonPressed;

		internal void OnUpdate()
		{
			// ---- Moving ----
			IsLeftKeyPressed = Input.IsKeyDown(m_MoveLeftKeyCode);
			IsRightKeyPressed = Input.IsKeyDown(m_MoveRightKeyCode);
			IsUpKeyPressed = Input.IsKeyDown(m_MoveUpKeyCode);
			IsDownKeyPressed = Input.IsKeyDown(m_MoveDownKeyCode);

			// ---- Shooting ----
			m_WasShootMouseButtonPressed = m_IsShootKeyReleased;
			m_IsShootMouseButtonPressed = Input.IsMouseButtonDown(m_ShootMouseCode);
			m_IsShootKeyReleased = Input.IsMouseButtonUp(m_ShootMouseCode);
		}
	}
}
