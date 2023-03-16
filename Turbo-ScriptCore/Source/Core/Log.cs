namespace Turbo
{
	internal enum LogLevel : uint
	{
		Info = 0,
		Warn,
		Error,
		Fatal
	}
	public static class Log
	{
		public static void Info(string message)
		{
			InternalCalls.Log_String(LogLevel.Info, message);
		}

		public static void Warn(string message)
		{
			InternalCalls.Log_String(LogLevel.Warn, message);
		}

		public static void Error(string message)
		{
			InternalCalls.Log_String(LogLevel.Error, message);
		}

		public static void Fatal(string message)
		{
			InternalCalls.Log_String(LogLevel.Fatal, message);
		}
	}
}
