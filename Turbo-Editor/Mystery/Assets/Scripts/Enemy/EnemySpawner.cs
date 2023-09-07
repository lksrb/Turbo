using System.Collections.Generic;
using Turbo;

namespace Mystery
{
	internal class EnemySpawner : Entity
	{
		Prefab m_EnemyPrefab;

		List<Enemy> m_Enemies;

		Timer m_SpawnTimer;

		protected override void OnCreate()
		{
			m_EnemyPrefab = Assets.LoadPrefab("Prefabs/Enemy.tprefab");

			m_Enemies = new List<Enemy>();

			m_SpawnTimer = new Timer(5.0f);
		}

		protected override void OnUpdate()
		{
			if(m_SpawnTimer)
			{
				SpawnEnemy();
			}
		}

		private void SpawnEnemy()
		{
			m_Enemies.Add(Instantiate(m_EnemyPrefab, Transform.Translation).As<Enemy>());
		}
	}
}
