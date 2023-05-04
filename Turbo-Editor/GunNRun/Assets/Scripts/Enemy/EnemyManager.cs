using Turbo;

namespace GunNRun
{
	public class EnemyManager : Entity
	{
		protected override void OnCreate()
		{
			Log.Info("Hello from enemy manager!");



			//foreach (Entity entity : GetChildren())
			//{
			//
			//}

			Instantiate("Assets/Prefabs/Enemy.tprefab", new Vector3(0, 10, 0));
		}

		protected override void OnUpdate(float ts)
		{
		}
	}
}
