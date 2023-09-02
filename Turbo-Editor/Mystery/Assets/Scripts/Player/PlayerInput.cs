using Turbo;

namespace Mystery
{
	internal class PlayerInput : PlayerModule
	{
		const MouseCode m_SetDestinationCode = MouseCode.ButtonLeft;
		const MouseCode m_FocusEnemyButtonDown = MouseCode.ButtonRight;
		const KeyCode m_ShootCode = KeyCode.W;
		const KeyCode m_PickupCode = KeyCode.F;

		internal bool IsSetDestinationButtonDown = false;
		internal bool IsFocusButtonDown = false;
		
		internal bool IsPickUpButtonDown = false;

		private bool m_IsShootKeyReleased = true;
		private bool m_WasShootMouseButtonPressed = false;
		private bool m_IsShootMouseButtonPressed = false;
		internal bool IsShootMouseButtonPressed => m_WasShootMouseButtonPressed && m_IsShootMouseButtonPressed;

		protected override void OnUpdate()
		{
			IsSetDestinationButtonDown = Input.IsMouseButtonDown(m_SetDestinationCode);
			IsFocusButtonDown = Input.IsMouseButtonDown(m_FocusEnemyButtonDown);
			IsPickUpButtonDown = Input.IsKeyDown(m_PickupCode);

			// Ball shoot
			m_WasShootMouseButtonPressed = m_IsShootKeyReleased;
			m_IsShootMouseButtonPressed = Input.IsKeyDown(m_ShootCode);
			m_IsShootKeyReleased = Input.IsKeyUp(m_ShootCode);
		}
	}
}
