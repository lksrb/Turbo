using Turbo;

namespace GunNRun
{
	internal class LevelManager
	{
		private GameManager m_GameManager;
		private Entity m_SpawnTopBoundary, m_SpawnBottomBoundary, m_SpawnLeftBoundary, m_SpawnRightBoundary;
		private Timer m_AnotherWaveTimer = new Timer(4.0f);

		internal void Init(GameManager game)
		{
			m_GameManager = game;

			m_SpawnTopBoundary = game.FindEntityByName("SpawnTopBoundary");
			m_SpawnBottomBoundary = game.FindEntityByName("SpawnBottomBoundary");
			m_SpawnLeftBoundary = game.FindEntityByName("SpawnLeftBoundary");
			m_SpawnRightBoundary = game.FindEntityByName("SpawnRightBoundary");

			SpawnAtRandomLocation(EnemyType.Shooter);
		}

		internal void OnUpdate()
		{

		}


		private void SpawnAtRandomLocation(EnemyType type)
		{
			Vector2 random = Vector2.Zero;
			random.X = Random.Float(m_SpawnLeftBoundary.Transform.Translation.X, m_SpawnRightBoundary.Transform.Translation.X);
			random.Y = Random.Float(m_SpawnTopBoundary.Transform.Translation.Y, m_SpawnBottomBoundary.Transform.Translation.Y);

			m_GameManager.Enemies.SpawnEnemy(random, type);
		}
	}
}
