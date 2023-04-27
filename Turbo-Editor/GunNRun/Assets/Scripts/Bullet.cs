using Turbo;

namespace GunNRun
{
	public class Bullet : Entity
    {
        private float m_DeathTimer = 1.0f;
        private bool m_OneTime = false;

		protected override void OnCreate()
        {
            Log.Info("Hello from bullet!");
        }

        protected override void OnUpdate(float ts)
        {
            if(!m_OneTime && m_DeathTimer < 0.0f)
            {
                m_OneTime = true;
                Scene.DestroyEntity(this);
            }
            m_DeathTimer -= ts;
        }
    }
}
