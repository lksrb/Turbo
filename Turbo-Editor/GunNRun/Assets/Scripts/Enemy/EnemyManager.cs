using System;
using System.Collections.Generic;
using Turbo;

namespace GunNRun
{
	public class EnemyManager : Entity
	{
		private readonly string m_EnemyPrefab = "Assets/Prefabs/Enemy.tprefab";
		private List<Entity> m_Enemies;
		private Entity[] m_EntitySpawnPoints;

		protected override void OnCreate()
		{
			m_EntitySpawnPoints = GetChildren();
			m_Enemies = new List<Entity>(m_EntitySpawnPoints.Length);

			for (int i = 0; i < 1; i++)
			{
				var translation = m_EntitySpawnPoints[i].Transform.Translation;
				translation.Z = 1.0f;
				m_Enemies.Add(Instantiate(m_EnemyPrefab, translation));
			}
		}

		protected override void OnUpdate(float ts)
		{

		}
	}
}
