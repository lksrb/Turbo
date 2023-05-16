using Turbo;

namespace GunNRun
{
	public enum GameCategory : uint
	{
		Bullet = 1 << 1,
		Player = 1 << 2,
		Enemy = 1 << 3,
		Wall = 1 << 4,
		Everything = 0xFFFF
	}

	internal class GameManager : Entity
	{
		private Entity m_SpawnTopBoundary, m_SpawnBottomBoundary, m_SpawnLeftBoundary, m_SpawnRightBoundary;
		private Timer m_AnotherWaveTimer = new Timer(4.0f);

		internal EnemyManager Enemies = new EnemyManager();

		protected override void OnCreate()
		{
			m_SpawnTopBoundary = FindEntityByName("SpawnTopBoundary");
			m_SpawnBottomBoundary = FindEntityByName("SpawnBottomBoundary");
			m_SpawnLeftBoundary = FindEntityByName("SpawnLeftBoundary");
			m_SpawnRightBoundary = FindEntityByName("SpawnRightBoundary");

			// Input.SetCursorMode(CursorMode.Hidden);

			Enemies.Init(this);

			SpawnAtRandomLocation(EnemyType.Shooter);
		}

		protected override void OnUpdate()
		{

		}

		private void SpawnAtRandomLocation(EnemyType type)
		{
			Vector2 random = Vector2.Zero;
			random.X = Random.Float(m_SpawnLeftBoundary.Transform.Translation.X, m_SpawnRightBoundary.Transform.Translation.X);
			random.Y = Random.Float(m_SpawnTopBoundary.Transform.Translation.Y, m_SpawnBottomBoundary.Transform.Translation.Y);

			Enemies.SpawnEnemy(random, type);
		}
	}
}
