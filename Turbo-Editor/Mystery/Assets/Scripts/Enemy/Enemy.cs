﻿using Turbo;

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
		public readonly float PickUpRadius = 0.0f;

		private Vector3 m_Forward = Vector3.Zero; 
		internal Vector3 Forward => m_Forward; 

		private System.Action<Entity> m_OnHitCallback = null;

		IEnemyState[] m_EnemyStates;
		IEnemyState m_CurrentState;
		EnemyState m_ECurrentState = EnemyState.Count;

		protected override void OnCreate()
		{
			EnemyManager.RegisterEnemy(this);
			OnCollisionBegin += Enemy_OnCollisionBegin;

			m_EnemyStates = new IEnemyState[(uint)EnemyState.Count];
			m_EnemyStates[(uint)EnemyState.ChaseBall] = new EnemyChaseBallState(this);
			m_EnemyStates[(uint)EnemyState.Attack] = new EnemyAttackState(this);
			m_EnemyStates[(uint)EnemyState.ExhaustedByAttack] = new EnemyExhaustedByAttack(this);
			m_EnemyStates[(uint)EnemyState.RunAway] = new EnemyRunAwayState(this);
			m_EnemyStates[(uint)EnemyState.Defend] = new EnemyDefendState(this);
			m_EnemyStates[(uint)EnemyState.TryCatchBall] = new EnemyTryCatchBallState(this);

			ChangeState(EnemyState.ChaseBall);
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

			m_CurrentState = m_EnemyStates[(uint)m_ECurrentState];
			m_CurrentState.Enter();
		}

		private void Enemy_OnCollisionBegin(Entity entity)
		{
			m_OnHitCallback?.Invoke(entity);
		}

		internal void OnEnemyEvent(Enemy other, EnemyEvent e)
		{

		}

		internal void OnPlayerEvent(PlayerEvent e)
		{
			m_CurrentState.OnPlayerEvent(e);
		}
	}
}
