using Turbo;

namespace GunNRun
{
	public class Bullet : Entity
    {
		public float Speed;

        private float m_DeathTimer = 1.0f;
        private bool m_OneTime = false;

		private bool m_Destroy = false;

		protected override void OnCreate()
        {
			Log.Info("Hello from bullet!");

			OnCollisionBegin2D += OnCollisionBegin;
		}

		protected override void OnUpdate(float ts)
        {
			if ((!m_OneTime && m_DeathTimer < 0.0f) || m_Destroy)
            {
                m_OneTime = true;
                Scene.DestroyEntity(this);
            }
            m_DeathTimer -= ts;
        }

		internal void SetDirection(Vector2 direction)
		{
			var rigidBody = GetComponent<Rigidbody2DComponent>();
			rigidBody.Velocity = direction * Speed;
		}

		private void OnCollisionBegin(Entity other)
		{
			m_Destroy = true;
		}
	}
}
