namespace Turbo
{
	public enum CursorMode : uint
	{
		Hidden = 0,
		Arrow,
		Hand,
	}

	public static class Input
	{
		public static bool IsKeyPressed(KeyCode code) => InternalCalls.Input_IsKeyPressed(code);
		public static bool IsKeyReleased(KeyCode code) => InternalCalls.Input_IsKeyReleased(code);
		public static bool IsMouseButtonPressed(MouseCode code) => InternalCalls.Input_IsMouseButtonPressed(code);
		public static bool IsMouseButtonReleased(MouseCode code) => InternalCalls.Input_IsMouseButtonReleased(code);

		public static void SetCursorMode(CursorMode cursorMode) => InternalCalls.Input_SetCursorMode(cursorMode);

		public static Vector2 MousePosition 
		{	
			get
			{
				InternalCalls.Input_GetMousePosition(out Vector2 mousePosition);
				return mousePosition;
			}
		}


	}
}
