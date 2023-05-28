using System;
using System.Collections.Generic;
using Turbo;

namespace GunNRun
{
	internal enum LevelState : uint
	{
		WaitingForNextWave = 0,
		Wave,
		WaveFinished
	}

	internal class LevelManager : IGameManagerModule
	{
		private GameManager m_GameManager;
		private Vector3 m_SpawnTopLeftBoundary, m_SpawnBottomRightBoundary;
		private LevelState m_CurrentState = LevelState.WaitingForNextWave;
		private List<Entity> m_CurrentEnemies = new List<Entity>(10);
		private int m_CurrentLevel = 1;
		private SingleTickTimer m_AfterWaveClearedTimer = new SingleTickTimer(1.0f);

		internal event Action<LevelState> OnChangeLevelState;

		void IGameManagerModule.Init(GameManager manager)
		{
			m_GameManager = manager;

			m_SpawnTopLeftBoundary = manager.FindEntityByName("SpawnTopLeftBoundary").Transform.Translation;
			m_SpawnBottomRightBoundary = manager.FindEntityByName("SpawnBottomRightBoundary").Transform.Translation;
		}

		void IGameManagerModule.OnUpdate()
		{
			switch (m_CurrentState)
			{
				case LevelState.WaitingForNextWave:
					break;
				case LevelState.Wave:
					if (m_CurrentEnemies.Count == 0)
					{
						m_AfterWaveClearedTimer.Reset();
						ChangeLevelState(LevelState.WaveFinished);
					}
					break;
				case LevelState.WaveFinished:
					if (m_AfterWaveClearedTimer)
					{
						ChangeLevelState(LevelState.WaitingForNextWave);
					}

					break;
			}
		}

		void IGameManagerModule.OnNewWave()
		{
			EnemySpawner.BeginSpawn(m_SpawnTopLeftBoundary, m_SpawnBottomRightBoundary);
			EnemySpawner.SpawnEnemyRandom(m_GameManager);
			EnemySpawner.SpawnEnemyRandom(m_GameManager);
			EnemySpawner.SpawnEnemyRandom(m_GameManager);
			EnemySpawner.SpawnEnemyRandom(m_GameManager);
			EnemySpawner.SpawnEnemyRandom(m_GameManager);
			EnemySpawner.SpawnEnemyRandom(m_GameManager);
			EnemySpawner.SpawnEnemyRandom(m_GameManager);
			EnemySpawner.EndSpawn();

			ChangeLevelState(LevelState.Wave);
		}

		private void ChangeLevelState(LevelState state)
		{
			if (state == m_CurrentState)
				return;

			m_CurrentState = state;

			switch (m_CurrentState)
			{
				case LevelState.WaitingForNextWave:
					break;
				case LevelState.Wave:
					break;
				case LevelState.WaveFinished:
					m_CurrentLevel++;
					break;
			}

			OnChangeLevelState?.Invoke(m_CurrentState);
		}

		internal void OnEnemyCreated(Entity enemy)
		{
			m_CurrentEnemies.Add(enemy);
		}

		internal void OnEnemyDestroyed(Entity enemy)
		{
			if (m_CurrentEnemies.Remove(enemy))
			{
				Log.Info("Enemy destroyed!");
			}
		}

		internal int CurrentLevel => m_CurrentLevel;
		internal List<Entity> CurrentEnemies => m_CurrentEnemies;
	}
}
