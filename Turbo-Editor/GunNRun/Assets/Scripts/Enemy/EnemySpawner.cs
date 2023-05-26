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

	internal class EnemySpawner
	{
		private struct SpawnStack
		{
			public Vector2 Min;
			public Vector2 Max;
		}

		private static EnemySpawner s_Instance = null;
		internal static EnemySpawner Instance
		{
			get
			{
				if (s_Instance == null)
					s_Instance = new EnemySpawner();

				return s_Instance;
			}
		}

		private string[] m_PrefabPaths;

		private bool m_BeginCalled = false;
		private SpawnStack m_SpawnStack;

		private EnemySpawner()
		{
			m_PrefabPaths = new string[(uint)EnemyType.Count];
			m_PrefabPaths[(uint)EnemyType.Suicider] = "Assets/Prefabs/Suicider.tprefab";
			m_PrefabPaths[(uint)EnemyType.Shooter] = "Assets/Prefabs/Shooter.tprefab";
			m_PrefabPaths[(uint)EnemyType.RPG] = "Assets/Prefabs/RPG.tprefab";
		}

		internal static void BeginSpawn(Vector2 min, Vector2 max)
		{
			Instance.m_BeginCalled = true;

			Instance.m_SpawnStack.Min = min;
			Instance.m_SpawnStack.Max = max;
		}

		internal static void EndSpawn()
		{
			Instance.m_BeginCalled = false;
		}

		internal static Entity SpawnEnemy(GameManager manager, Vector2 translation, EnemyType type)
		{
			Entity entity = manager.Instantiate(Instance.m_PrefabPaths[(uint)type], new Vector3(translation, 0.35f));
			manager.GetLevelManager().OnEnemyCreated(entity);
			return entity;
		}

		internal static Entity SpawnEnemyRandom(GameManager manager, EnemyType type = EnemyType.Count)
		{
			if(!Instance.m_BeginCalled)
			{
				Log.Info("Call BeginSpawn()!");
				return null;
			}

			if(type == EnemyType.Count)
			{
				type = (EnemyType)Random.Int((int)EnemyType.Suicider, (int)EnemyType.Count);
			}

			type = EnemyType.Shooter;
				
			Vector2 randomLocation = Vector2.Zero;
			randomLocation.X = Random.Float(Instance.m_SpawnStack.Min.X, Instance.m_SpawnStack.Max.X);
			randomLocation.Y = Random.Float(Instance.m_SpawnStack.Min.Y, Instance.m_SpawnStack.Max.Y);
			return SpawnEnemy(manager, randomLocation, type);
		}
	}
}
