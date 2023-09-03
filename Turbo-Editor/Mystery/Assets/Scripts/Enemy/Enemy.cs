using Turbo;
using static Mystery.Layer<Mystery.Enemy, Mystery.EnemyEvent>;

namespace Mystery
{
	enum EnemyEvent : uint
	{
		ChaseBall = 0,
		RunFromPlayer,
		BallGrabbed,
	}

	enum EnemyTarget : uint
	{
		TargetBall = 0,
		TargetPlayer
	}

	internal interface EnemyManagerObserver
	{
		void OnEnemyEvent(Enemy enemy, EnemyEvent enemyEvent);
		void OnPlayerEvent(PlayerEvent playerEvent);
	}

	internal class Enemy : Entity
	{
		public readonly float Speed;
		public readonly float PickUpRadius;

		private LayerSystem<Enemy, EnemyEvent> m_LayerSystem;
		private System.Action<Entity> m_OnHitCallback;
		private EnemyLayer m_EnemyLayer;

		protected override void OnCreate()
		{
			EnemyManager.RegisterEnemy(this);

			OnCollisionBegin += Enemy_OnCollisionBegin;

			m_LayerSystem = new LayerSystem<Enemy, EnemyEvent>(this, 1);

			m_EnemyLayer = m_LayerSystem.PushLayer<EnemyLayer>();
			m_OnHitCallback += m_LayerSystem.PushLayer<EnemyMovement>().OnHit;
			m_LayerSystem.PushLayer<EnemyBallThrow>();

			m_LayerSystem.Listen<EnemyMovement>().To<EnemyLayer>();
			m_LayerSystem.Listen<EnemyMovement>().To<EnemyBallThrow>();
		}

		protected override void OnUpdate()
		{
			m_LayerSystem.OnUpdate();
		}

		internal EnemyLayer GetEnemyLayer() => m_EnemyLayer;
		private void Enemy_OnCollisionBegin(Entity entity) => m_OnHitCallback?.Invoke(entity);
	}

	// EnemyLayer
	// EnemyLayer
	// EnemyLayer

	internal class EnemyLayer : Layer<Enemy, EnemyEvent>, EnemyManagerObserver
	{
		public void OnPlayerEvent(PlayerEvent playerEvent)
		{
			if (playerEvent == PlayerEvent.BallGrabbed)
			{
				Emit(EnemyEvent.RunFromPlayer);
			} 
			else if(playerEvent == PlayerEvent.BallThrew)
			{
				Emit(EnemyEvent.ChaseBall);
			}
		}

		public void OnEnemyEvent(Enemy enemy, EnemyEvent enemyEvent)
		{
		}
	}
}
