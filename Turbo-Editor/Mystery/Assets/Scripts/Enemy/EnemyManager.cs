using System.Collections.Generic;

namespace Mystery
{
	internal class EnemyManager
	{
		private static EnemyManager s_Instance;
		internal static EnemyManager Get() => s_Instance;
		internal EnemyManager() => s_Instance = this;
		~EnemyManager() => s_Instance = null;
		// ----

		private List<Enemy> m_Enemies = new List<Enemy>(10);

		internal static void RegisterEnemy(Enemy enemy)
		{
			Get().m_Enemies.Add(enemy);
		}

		internal static void UnregisterEnemy(Enemy enemy)
		{
			Get().m_Enemies.Remove(enemy);
		}

		internal static void Emit(Enemy emitter, EnemyEvent e)
		{
			foreach(var enemy in Get().m_Enemies)
			{
				if (emitter == enemy)
					continue;

				enemy.OnEnemyEvent(emitter, e);
			}
		}
	}
}
