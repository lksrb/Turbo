using System;
using System.Collections.Generic;
using Turbo;

namespace GunNRun
{
	internal enum EnemyType : uint
	{
		Suicider = 0,
		Shooter,
		RPG,

		Count
	}

	internal class EnemyManager
	{
		private List<Entity> m_Enemies = new List<Entity>(10);
		private GameManager m_GameManager;

		private string[] m_PrefabPaths;

		internal void Init(GameManager manager)
		{
			m_GameManager = manager;

			m_PrefabPaths = new string[(uint)EnemyType.Count];
			m_PrefabPaths[(uint)EnemyType.Suicider] = "Assets/Prefabs/Suicider.tprefab";
			m_PrefabPaths[(uint)EnemyType.Shooter] = "Assets/Prefabs/Shooter.tprefab";
			m_PrefabPaths[(uint)EnemyType.RPG] = "Assets/Prefabs/RPG.tprefab";
		}

		internal void SpawnEnemy(Vector3 translation, EnemyType type)
		{
			Entity enemy = m_GameManager.Instantiate(m_PrefabPaths[(uint)type], translation);
			m_Enemies.Add(enemy);
		}

		internal List<Entity> Enemies => m_Enemies;
	}
}
