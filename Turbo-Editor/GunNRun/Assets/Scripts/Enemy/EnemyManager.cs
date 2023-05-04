using Turbo;

namespace GunNRun
{
	public class EnemyManager : Entity
	{
		public float IdleAnimationSpeed;

		private Enemy[] m_Enemies;
		private string m_EnemyPrefabPath = "Assets/Prefabs/Enemy.tprefab";

		protected override void OnCreate()
		{
			Log.Info("Hello from enemy manager!");

			var children = Children();
			m_Enemies = new Enemy[children.Length];

			for (int i = 0; i < children.Length; i++)
			{
				m_Enemies[i] = Instantiate(m_EnemyPrefabPath, children[i].Transform.Translation).As<Enemy>();
			}
		}

		protected override void OnUpdate(float ts)
		{
		}
	}
}
