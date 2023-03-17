namespace Turbo
{
	public class ScriptCoreTest : Entity
	{
		void OnStart()
		{
			Log.Info("Hello from C#!");
			Log.Warn("Hello from C#!");

			Log.Info($"Entity ID: {ID}!");

			Log.Info($"Entity transform: {transform.Translation.y}");
		}

		void OnUpdate(float ts)
		{
			if(Input.IsKeyPressed(KeyCode.A) || Input.IsMouseButtonPressed(MouseCode.ButtonLeft))
			{
				Log.Error($"Hello from C#!\n{ts}");
			}
		}
	}
}
