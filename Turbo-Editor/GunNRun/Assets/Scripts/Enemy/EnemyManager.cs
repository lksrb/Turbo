using Turbo;

namespace GunNRun
{
	public class EnemyManager : Entity
	{
		// ---- Enemy Animation ----
		public float IdleAnimationDelay;
		public float RunningAnimationDelay;
		public float IdleShootingAnimationDelay;
		public float RunShootingAnimationDelay;

		private Enemy[] m_Enemies;
		private string m_EnemyPrefabPath = "Assets/Prefabs/Enemy.tprefab";

		protected override void OnCreate()
		{
			Log.Info("Hello from enemy manager!");

			var children = Children();
			m_Enemies = new Enemy[children.Length];

			for (int i = 0; i < children.Length; i++)
			{
				Entity child = children[i];

				if(child.Name == "EnemySpawnpoint")
				{
					m_Enemies[i] = Instantiate(m_EnemyPrefabPath, child.Transform.Translation).As<Enemy>();
				}
			}
		}

		protected override void OnUpdate(float ts)
		{
		}
	}
}
