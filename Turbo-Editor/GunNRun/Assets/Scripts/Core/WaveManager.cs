using System;
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
		private GameManager m_GameManager;
		private Vector3 m_SpawnTopLeftBoundary, m_SpawnBottomRightBoundary;
		private WaveState m_CurrentState = WaveState.WaitingForNextWave;
		private List<Entity> m_CurrentEnemies = new List<Entity>(10);
		private int m_CurrentWave = 1;
		private SingleTickTimer m_AfterWaveClearedTimer = new SingleTickTimer(1.0f);

		// Wave timer
		private SingleTickTimer m_NewWaveTimer = new SingleTickTimer(4.0f);

		private EnemySpawner m_EnemySpawner;

		internal event Action<WaveState> OnChangeWaveState;

		internal WaveManager(GameManager manager)
		{
			m_GameManager = manager;

			m_SpawnTopLeftBoundary = manager.FindEntityByName("SpawnTopLeftBoundary").Transform.Translation;
			m_SpawnBottomRightBoundary = manager.FindEntityByName("SpawnBottomRightBoundary").Transform.Translation;

			m_EnemySpawner = new EnemySpawner(OnEnemyCreated, OnEnemyDestroyed);
		}

		internal void OnUpdate()
		{
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
						ChangeLevelState(WaveState.WaitingForNextWave);
						m_NewWaveTimer.Reset();
						m_CurrentWave++;
					}

					break;
			}

			if(m_NewWaveTimer)
			{
				SpawnNewWave();
			}
		}

		internal void SpawnNewWave()
		{
			{
				m_EnemySpawner.SetBounds(m_SpawnTopLeftBoundary, m_SpawnBottomRightBoundary);
				m_EnemySpawner.SpawnEnemyRandom(1);

				m_EnemySpawner.SetBounds(m_SpawnTopLeftBoundary, m_SpawnBottomRightBoundary);
				m_EnemySpawner.SpawnEnemyRandom(1);
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

		internal void OnEnemyCreated(Entity enemy)
		{
			m_CurrentEnemies.Add(enemy);

			Log.Info("Enemy added!");
		}

		internal void OnEnemyDestroyed(Entity enemy)
		{
			if (m_CurrentEnemies.Remove(enemy))
			{
				Log.Info("Enemy destroyed!");
			}
		}

		internal int CurrentWave => m_CurrentWave;
		internal List<Entity> CurrentEnemies => m_CurrentEnemies;
	}
}
