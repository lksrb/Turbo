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
		internal EnemyManager Enemies = new EnemyManager();

		protected override void OnCreate()
		{
			// Input.SetCursorMode(CursorMode.Hidden);

			Enemies.Init(this);
			//Enemies.SpawnEnemy(new Vector3(5, 5, 1), EnemyType.Shooter);
		}

		protected override void OnUpdate()
		{
		}
	}
}
