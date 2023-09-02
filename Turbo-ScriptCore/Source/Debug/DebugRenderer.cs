﻿namespace Turbo
{
	public static class DebugRenderer
	{
		public static void DrawLine(Vector3 start, Vector3 end, Color color)
		{
			InternalCalls.DebugRenderer_DrawLine(ref start, ref end, ref color);
		}

		public static void DrawCircle(Vector3 position, Vector3 rotation, float radius, Color color)
		{
			InternalCalls.DebugRenderer_DrawCircle(ref position, ref rotation, radius, ref color);
		}

		public static void DrawBox(Vector3 position, Vector3 rotation, Vector3 scale, Color color)
		{
			InternalCalls.DebugRenderer_DrawBox(ref position, ref rotation, ref scale, ref color);
		}
	}
}
