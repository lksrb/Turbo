using Turbo;

namespace Mystery
{
	internal enum EnemyState : uint
	{
		ChasePlayer = 0,
		Stunned,
		Dead,

		Count
	}

	internal abstract class EnemyStateBase
	{
		protected Enemy m_Enemy;
		protected RigidbodyComponent m_Rigidbody;
		protected Player m_Player;

		protected EnemyStateBase(Enemy enemy, Player player)
		{
			m_Enemy = enemy;

			m_Player = player;
			m_Rigidbody = m_Enemy.GetComponent<RigidbodyComponent>();
		}

		internal void SetCollisionCallbacks()
		{
			m_Enemy.OnCollisionBegin += OnCollisionBegin;
			m_Enemy.OnCollisionEnd += OnCollisionEnd;
		}

		internal void UnsetCollisionCallbacks()
		{
			m_Enemy.OnCollisionBegin -= OnCollisionBegin;
			m_Enemy.OnCollisionEnd -= OnCollisionEnd;
		}

		protected Entity FindEntityByName(string name) => m_Enemy.FindEntityByName(name);
		protected void ChangeState(EnemyState state) => m_Enemy.ChangeState(state);

		internal virtual void Enter() { }
		internal virtual void OnUpdate() { }

		protected virtual void OnCollisionBegin(Entity entity) { }
		protected virtual void OnCollisionEnd(Entity entity) { }

		internal static EnemyStateBase[] CreateEnemyStates(Enemy enemy, Player player)
		{
			EnemyStateBase[] states = new EnemyStateBase[(uint)EnemyState.Count];
			states[(uint)EnemyState.ChasePlayer] = new EnemyChasePlayerState(enemy, player);
			states[(uint)EnemyState.Stunned] = new EnemyStunnedState(enemy, player);
			states[(uint)EnemyState.Dead] = new EnemyDeadState(enemy, player);

			return states;
		}
	}
}
