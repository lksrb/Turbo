using System.Collections.Generic;
using Turbo;

namespace GunNRun
{
	internal enum WaveState : uint
	{
		WaitingForNextWave = 0,
		Wave,
		WaveFinished
	}

	internal class WaveManager
	{
		private readonly string[] m_DropItemsPrefabs = new string[2]
		{
			"Assets/Prefabs/HpDrop.tprefab",
			"Assets/Prefabs/AmmoDrop.tprefab"
		};

		private Vector3 m_SpawnTopLeftBoundary, m_SpawnBottomRightBoundary;
		private WaveState m_CurrentState = WaveState.WaitingForNextWave;
		private List<Entity> m_CurrentEnemies = new List<Entity>(20);
		private int m_CurrentWave = 1;
		private SingleTickTimer m_AfterWaveClearedTimer = new SingleTickTimer(1.0f);
		private bool m_SpawnEnemies = true;

		// Wave timer
		private SingleTickTimer m_NewWaveTimer = new SingleTickTimer(4.0f);
		private EnemySpawner m_EnemySpawner;

		internal event System.Action<WaveState> OnChangeWaveState;

		internal int CurrentWave => m_CurrentWave;
		internal List<Entity> CurrentEnemies => m_CurrentEnemies;

		private Player m_Player;

		internal WaveManager(GameManager manager)
		{
			m_Player = manager.FindEntityByName("Player").As<Player>();
			m_SpawnTopLeftBoundary = manager.FindEntityByName("SpawnTopLeftBoundary").Transform.Translation;
			m_SpawnBottomRightBoundary = manager.FindEntityByName("SpawnBottomRightBoundary").Transform.Translation;

			m_EnemySpawner = new EnemySpawner(m_Player, OnEnemyCreated, OnEnemyDestroyed);
		}

		internal void OnUpdate()
		{
			if (!m_SpawnEnemies)
				return;

			switch (m_CurrentState)
			{
				case WaveState.WaitingForNextWave:
					break;
				case WaveState.Wave:
					if (m_CurrentEnemies.Count == 0)
					{
						m_AfterWaveClearedTimer.Reset();
						ChangeLevelState(WaveState.WaveFinished);
					}
					break;
				case WaveState.WaveFinished:
					if (m_AfterWaveClearedTimer)
					{
						m_CurrentWave++;
						m_NewWaveTimer.Reset();

						ChangeLevelState(WaveState.WaitingForNextWave);
					}

					break;
			}

			if (m_NewWaveTimer)
			{
				SpawnNewWave();
			}
		}

		private void SpawnNewWave()
		{
			{
				m_EnemySpawner.SetBounds(m_SpawnTopLeftBoundary, m_SpawnBottomRightBoundary);
				m_EnemySpawner.SetMinPlayerDistance(5.0f);
				m_EnemySpawner.SetMinDistance(5.0f);
				m_EnemySpawner.SpawnEnemyRandom(m_CurrentWave + Random.Int(1, 3));
			}

			ChangeLevelState(WaveState.Wave);
		}

		private void ChangeLevelState(WaveState state)
		{
			if (state == m_CurrentState)
				return;

			m_CurrentState = state;

			OnChangeWaveState?.Invoke(m_CurrentState);
		}

		private void OnEnemyCreated(Entity enemy)
		{
			m_CurrentEnemies.Add(enemy);
		}

		private void OnEnemyDestroyed(Entity enemy)
		{
			if (m_CurrentEnemies.Remove(enemy))
			{
				m_Player.AddScore(enemy);

				// Spawning drops
				int dropRate = Random.Int(0, 10);
				if (dropRate < 4)
				{
					Log.Info(dropRate);
					dropRate = dropRate < 3 ? 0 : 1;

					Scene.InstantiateEntity(m_DropItemsPrefabs[dropRate], enemy.Transform.Translation);
				}
			}
		}
	}
}
