using Turbo;

namespace GunNRun
{
	public class Enemy : Entity
	{
		protected override void OnCreate()
		{
			Log.Info("Enemy spawned!");
		}

		protected override void OnUpdate(float ts)
		{
		}
	}
}
