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
		public static bool IsKeyDown(KeyCode code) => InternalCalls.Input_IsKeyDown(code);
		public static bool IsKeyUp(KeyCode code) => InternalCalls.Input_IsKeyUp(code);
		public static bool IsMouseButtonDown(MouseCode code) => InternalCalls.Input_IsMouseButtonDown(code);
		public static bool IsMouseButtonUp(MouseCode code) => InternalCalls.Input_IsMouseButtonUp(code);

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
