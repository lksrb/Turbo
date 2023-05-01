using Turbo;

namespace GunNRun
{
	public class EnemyManager : Entity
	{
		protected override void OnCreate()
		{
			Log.Info("Hello from enemy manager!");

			Instantiate("Assets/Prefabs/Enemy.tprefab", Vector3.Zero);
		}

		protected override void OnUpdate(float ts)
		{
		}
	}
}
