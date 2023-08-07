namespace Turbo
{
	public static class Application
	{
		public static uint GetWidth() => InternalCalls.Application_GetWidth();
		public static uint GetHeight() => InternalCalls.Application_GetHeight();
		public static void Close() => InternalCalls.Application_Close();
	}
}
