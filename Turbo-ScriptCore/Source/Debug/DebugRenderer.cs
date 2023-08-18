namespace Turbo
{
	public static class DebugRenderer
	{
		public static void DrawLine(Vector3 start, Vector3 end, Color color)
		{
			InternalCalls.DebugRenderer_DrawLine(ref start, ref end, ref color);
		}
	}
}
