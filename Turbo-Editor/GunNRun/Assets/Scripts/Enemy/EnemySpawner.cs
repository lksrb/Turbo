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
		private string[] m_PrefabPaths;

		private bool m_BeginCalled = false;
		private System.Action<Entity> m_OnSpawnEntity, m_OnDestroyEntity;

		private Vector2 m_Min = Vector2.Zero;
		private Vector2 m_Max = Vector2.Zero;

		public EnemySpawner(System.Action<Entity> onSpawnEntity, System.Action<Entity> onDestroyEntity)
		{
			m_OnSpawnEntity = onSpawnEntity;
			m_OnDestroyEntity = onDestroyEntity;

			m_PrefabPaths = new string[(uint)EnemyType.Count];
			m_PrefabPaths[(uint)EnemyType.Suicider] = "Assets/Prefabs/Suicider.tprefab";
			m_PrefabPaths[(uint)EnemyType.Shooter] = "Assets/Prefabs/Shooter.tprefab";
			m_PrefabPaths[(uint)EnemyType.RPG] = "Assets/Prefabs/RPG.tprefab";
		}

		internal void SetBounds(Vector2 min, Vector2 max)
		{
			m_BeginCalled = true;

			m_Min = min;
			m_Max = max;
		}

		internal void SpawnEnemyRandom(uint count = 1)
		{
			if (!m_BeginCalled)
			{
				Log.Info("Call BeginSpawn()!");
				return;
			}

			for (uint i = 0; i < count; ++i)
			{
				EnemyType type = (EnemyType)Random.Int((int)EnemyType.Suicider, (int)EnemyType.Count);

				Vector2 randomLocation = Vector2.Zero;
				randomLocation.X = Random.Float(m_Min.X, m_Max.X);
				randomLocation.Y = Random.Float(m_Min.Y, m_Max.Y);

				SpawnEnemy(randomLocation, type);
			}
		}

		internal void SpawnEnemyRandom(Vector2 translation)
		{
			EnemyType type = (EnemyType)Random.Int((int)EnemyType.Suicider, (int)EnemyType.Count);
			SpawnEnemy(translation, type);
		}

		internal void SpawnEnemyRandom(EnemyType type, uint count = 1)
		{
			for (uint i = 0; i < count; ++i)
			{
				Vector2 randomLocation = Vector2.Zero;
				randomLocation.X = Random.Float(m_Min.X, m_Max.X);
				randomLocation.Y = Random.Float(m_Min.Y, m_Max.Y);

				SpawnEnemy(randomLocation, type);
			}
		}

		internal void SpawnEnemy(Vector2 translation, EnemyType type)
		{
			type = EnemyType.Shooter; // TODO: REMOVE

			if(type >= EnemyType.Count)
			{
				Log.Error("Out of bounds enemy type!");
				return;
			}

			Entity entity = Scene.InstantiateEntity(m_PrefabPaths[(uint)type], new Vector3(translation, 0.35f));

			switch (type)
			{
				case EnemyType.Suicider:
					break;
				case EnemyType.Shooter:
					entity.As<ShooterEnemy>().SetOnDestroyCallback(m_OnDestroyEntity);
					break;
				case EnemyType.RPG:
					break;
			}

			m_OnSpawnEntity?.Invoke(entity);
		}
	}
}
