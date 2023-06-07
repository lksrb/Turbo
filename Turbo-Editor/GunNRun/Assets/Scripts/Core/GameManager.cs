using System.Collections.Generic;
using Turbo;

namespace GunNRun
{
	// Collision masking
	internal enum EntityCategory
	{
		Bullet = 1 << 1,
		Player = 1 << 2,
		Enemy = 1 << 3,
		Wall = 1 << 4,
		Everything = 0xFFFF
	}

	internal enum GameState
	{
		Playing = 0,
		GameOver
	}

	public class GameManager : Entity
	{
		internal WaveManager WaveManager;
		private GameUI m_GameUI;

		private Player m_Player;

		private GameState m_CurrentGameState = GameState.Playing;

		internal GameState CurrentGameState => m_CurrentGameState;

		protected override void OnCreate()
		{
			Input.SetCursorMode(CursorMode.Hidden);
			m_Player = FindEntityByName("Player").As<Player>();

			WaveManager = new WaveManager(this);

			m_GameUI = new GameUI(this);
		}

		protected override void OnUpdate()
		{
			m_GameUI.OnUpdate();

			if (m_Player.HP <= 0)
			{
				m_CurrentGameState = GameState.GameOver;

				return;
			}

			WaveManager.OnUpdate();
		}
	}
}
