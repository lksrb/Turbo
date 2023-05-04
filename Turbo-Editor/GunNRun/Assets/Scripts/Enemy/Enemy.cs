using Turbo;

namespace GunNRun
{
	public class Enemy : Entity
	{
		private Rigidbody2DComponent m_Rigidbody2D;
		private EnemyAnimator m_Animator = new EnemyAnimator();
		internal EnemyManager m_EnemyManager;

		protected override void OnCreate()
		{
			Log.Info("Enemy spawned!");

			m_Rigidbody2D = GetComponent<Rigidbody2DComponent>();
			m_EnemyManager = FindEntityByName("EnemyManager").As<EnemyManager>();

			m_Animator.Init(this);
		}

		protected override void OnUpdate(float ts)
		{
			m_Animator.OnUpdate(ts);
		}
	}
}
