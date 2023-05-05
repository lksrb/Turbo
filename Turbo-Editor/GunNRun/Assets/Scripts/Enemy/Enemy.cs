using Turbo;

namespace GunNRun
{
	public class Enemy : Entity
	{
		private Rigidbody2DComponent m_Rigidbody2D;
		private EnemyAnimator m_Animator = new EnemyAnimator();
		private float m_Flip = 10.0f;

		internal EnemyManager m_EnemyManager;

		protected override void OnCreate()
		{
			Log.Info("Enemy spawned!");

			m_Rigidbody2D = GetComponent<Rigidbody2DComponent>();
			m_EnemyManager = FindEntityByName("EnemyManager").As<EnemyManager>();

			m_Animator.Init(this);

			OnCollisionBegin2D += OnCollision;
		}

		protected override void OnUpdate(float ts)
		{
			m_Animator.OnUpdate(ts);

			m_Rigidbody2D.Velocity = new Vector2(m_Flip, m_Rigidbody2D.Velocity.Y);
		}

		private void OnCollision(Entity other)
		{
			m_Flip *= -1;
		}
	}
}
