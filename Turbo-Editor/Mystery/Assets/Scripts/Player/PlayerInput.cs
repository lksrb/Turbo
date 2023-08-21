using Turbo;

namespace Mystery
{
	internal class PlayerInput
	{
		const MouseCode m_SetDestinationCode = MouseCode.ButtonRight;
		const MouseCode m_FocusEnemyButtonDown = MouseCode.ButtonLeft;
		const KeyCode m_ShootCode = KeyCode.W;
		const KeyCode m_PickupCode = KeyCode.F;

		internal bool IsSetDestinationButtonDown = false;
		internal bool IsFocusButtonDown = false;
		
		internal bool IsPickUpButtonDown = false;

		private bool m_IsShootKeyReleased = true;
		private bool m_WasShootMouseButtonPressed = false;
		private bool m_IsShootMouseButtonPressed = false;
		internal bool IsShootMouseButtonPressed => m_WasShootMouseButtonPressed && m_IsShootMouseButtonPressed;

		internal void Update()
		{
			IsSetDestinationButtonDown = Input.IsMouseButtonDown(m_SetDestinationCode);
			IsFocusButtonDown = Input.IsMouseButtonDown(m_FocusEnemyButtonDown);
			IsPickUpButtonDown = Input.IsKeyDown(m_PickupCode);

			// Shooting
			m_WasShootMouseButtonPressed = m_IsShootKeyReleased;
			m_IsShootMouseButtonPressed = Input.IsKeyDown(m_ShootCode);
			m_IsShootKeyReleased = Input.IsKeyUp(m_ShootCode);
		}
	}
}
