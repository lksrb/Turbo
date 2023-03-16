namespace Turbo
{
	public class ScriptCoreTest : Entity
	{
		void OnStart()
		{
			Log.Info("Hello from C#!");
			Log.Warn("Hello from C#!");
		}

		void OnUpdate(float f)
		{
			Log.Error($"Hello from C#!{f}");
			Log.Fatal("Hello from C#!");
		}
	}
}
