using Turbo;

namespace GunNRun
{
	internal enum EnemyType : int
	{
		Suicider = 0,
		Shooter,
		Sniper,

		Count
	}

	internal class EnemySpawner
	{
		private readonly string[] m_PrefabPaths = new string[(int)EnemyType.Count]
		{
			"Assets/Prefabs/Suicider.tprefab",
			"Assets/Prefabs/Shooter.tprefab",
			"Assets/Prefabs/Sniper.tprefab",
		};

		private System.Action<Entity> m_OnSpawnEntity, m_OnDestroyEntity;

		private Vector2 m_Min = Vector2.Zero;
		private Vector2 m_Max = Vector2.Zero;
		private float m_Distance = 0.0f;
		private Vector3 m_PlayerTranslation = Vector3.Zero;
		private float m_PlayerDistance = 0.0f;

		public EnemySpawner(System.Action<Entity> onSpawnEntity, System.Action<Entity> onDestroyEntity)
		{
			m_OnSpawnEntity = onSpawnEntity;
			m_OnDestroyEntity = onDestroyEntity;
		}

		internal void SetBounds(Vector2 min, Vector2 max)
		{
			m_Min = min;
			m_Max = max;
		}

		internal void SetPlayer(Vector3 translation, float distance)
		{
			m_PlayerTranslation = translation;
			m_PlayerDistance = distance;
		}

		internal void SetMinDistance(float distance)
		{
			m_Distance = distance;
		}

		internal void SpawnEnemyRandom(int count = 1)
		{
			var randomLocations = GenerateRandomLocations(count);

			for (int i = 0; i < count; ++i)
			{
				SpawnEnemyRandom(randomLocations[i]);
			}
		}

		internal void SpawnEnemyRandom(Vector2 translation, int count = 1)
		{
			for (int i = 0; i < count; ++i)
			{
				SpawnEnemy(translation, RandomType());
			}
		}

		internal void SpawnEnemyRandom(EnemyType type, int count = 1)
		{
			var randomLocations = GenerateRandomLocations(count);

			for (int i = 0; i < count; ++i)
			{
				SpawnEnemy(randomLocations[i], type);
			}
		}

		internal void SpawnEnemy(Vector2 translation, EnemyType type)
		{
			if (type >= EnemyType.Count)
			{
				Log.Error("Out of bounds enemy type!");
				return;
			}

			Entity entity = Scene.InstantiateEntity(m_PrefabPaths[(int)type], new Vector3(translation, 0.35f));

			switch (type)
			{
				case EnemyType.Suicider:
					entity.As<SuiciderEnemy>().SetOnDestroyCallback(m_OnDestroyEntity);
					break;
				case EnemyType.Shooter:
					entity.As<ShooterEnemy>().SetOnDestroyCallback(m_OnDestroyEntity);
					break;
				case EnemyType.Sniper:
					entity.As<SniperEnemy>().SetOnDestroyCallback(m_OnDestroyEntity);
					break;
			}

			m_OnSpawnEntity?.Invoke(entity);
		}

		private EnemyType RandomType(EnemyType minType = EnemyType.Suicider, EnemyType maxType = EnemyType.Count)
		{
			return (EnemyType)Random.Int((int)minType, (int)maxType);
		}

		private Vector2[] GenerateRandomLocations(int count)
		{
			Vector2[] randomLocations = new Vector2[count];

			for (int i = 0; i < count; ++i)
			{
				Vector2 randomLocation = Vector2.Zero;
				randomLocation.X = Random.Float(m_Min.X, m_Max.X);
				randomLocation.Y = Random.Float(m_Min.Y, m_Max.Y);

				bool foundCollision = false;
				for (int j = 0; j < i; ++j)
				{
					if (Mathf.Length(randomLocations[j] - randomLocation) < m_Distance)
					{
						foundCollision = true;
						break;
					}
				}

				float length = Mathf.Length(m_PlayerTranslation - randomLocation);
				if (length < m_PlayerDistance)
				{
					foundCollision = true;
				}

				if (foundCollision)
				{
					i--;
					continue;
				}

				randomLocations[i] = randomLocation;
			}

			return randomLocations;
		}
	}
}
