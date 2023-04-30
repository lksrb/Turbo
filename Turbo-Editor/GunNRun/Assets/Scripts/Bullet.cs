using Turbo;

namespace GunNRun
{
	public class Bullet : Entity
    {
		public float Speed;

        private float m_DeathTimer = 1.0f;
        private bool m_OneTime = false;

		protected override void OnCreate()
        {
			Log.Info("Hello from bullet!");
		}

		protected override void OnUpdate(float ts)
        {
			if (!m_OneTime && m_DeathTimer < 0.0f)
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
	}
}
