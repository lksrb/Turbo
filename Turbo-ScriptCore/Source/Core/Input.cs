namespace Turbo
{
	public static class Input
	{
		public static bool IsKeyPressed(KeyCode code)
		{
			return InternalCalls.Input_IsKeyPressed(code);
		}
		public static bool IsKeyReleased(KeyCode code)
		{
			return InternalCalls.Input_IsKeyReleased(code);
		}
		public static bool IsMouseButtonPressed(MouseCode code)
		{
			return InternalCalls.Input_IsMouseButtonPressed(code);
		}
		public static bool IsMouseButtonReleased(MouseCode code)
		{
			return InternalCalls.Input_IsMouseButtonReleased(code);
		}

		public static Vector2 MousePosition 
		{	
			get 
			{
				Vector2 mousePosition = new Vector2
				{
					X = InternalCalls.Input_GetMouseX(),
					Y = InternalCalls.Input_GetMouseY()
				};

				return mousePosition;
			}
		}
	}
}
