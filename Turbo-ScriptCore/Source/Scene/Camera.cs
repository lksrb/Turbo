using Turbo;

namespace Turbo
{
	public static class Camera
	{
		public static Vector3 ScreenToWorldPosition(Vector2 screenPosition)
		{
			InternalCalls.Scene_ScreenToWorldPosition(screenPosition, out Vector3 worldPosition);
			return worldPosition;
		}

		public static Vector2 WorldToScreenPosition(Vector3 worldPosition)
		{
			InternalCalls.Scene_WorldToScreenPosition(worldPosition, out Vector2 screenPosition);
			return screenPosition;
		}
	}
}
