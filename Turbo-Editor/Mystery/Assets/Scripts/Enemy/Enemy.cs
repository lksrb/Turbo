using Turbo;

namespace Mystery
{
	enum EnemyEvent : uint
	{
		ChaseBall = 0,
		RunFromPlayer,
		BallGrabbed,
	}

	internal class Enemy : Entity
	{
		public readonly float Speed = 0.0f;

		private Vector3 m_Forward = Vector3.Zero;
		internal Vector3 Forward => m_Forward;

		Player m_Player;
		EnemyStateBase[] m_EnemyStates;
		EnemyStateBase m_CurrentState = null;
		EnemyState m_ECurrentState = EnemyState.Count;

		protected override void OnCreate()
		{
			Log.Info("Enemy instantiated!");

			EnemyManager.RegisterEnemy(this);
			m_Player = FindEntityByName("Player").As<Player>();

			m_EnemyStates = EnemyStateBase.CreateEnemyStates(this, m_Player);

			ChangeState(EnemyState.ChasePlayer);
		}

		protected override void OnUpdate()
		{
			m_Forward = new Quaternion(Transform.Rotation) * Vector3.Forward;
			m_Forward.Y = 0.0f;
			m_Forward.Normalize();

			m_CurrentState.OnUpdate();
		}

		internal void ChangeState(EnemyState state)
		{
			if (m_ECurrentState == state)
				return;

			m_ECurrentState = state;

			if (m_CurrentState != null)
				m_CurrentState.UnsetCollisionCallbacks();

			m_CurrentState = m_EnemyStates[(uint)m_ECurrentState];
			m_CurrentState.SetCollisionCallbacks();
			m_CurrentState.Enter();
		}

		internal void OnEnemyEvent(Enemy other, EnemyEvent e)
		{

		}
	}
}
