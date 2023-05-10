using Turbo;

namespace GunNRun
{
	public class Bullet : Entity
    {
		public float Speed = 0.0f;
		public bool DestroyOnImpact = false;

        private float m_DeathTimer = 1.0f;
        private bool m_OneTime = false;

		private bool m_Destroy = false;
		private Rigidbody2DComponent m_Rigidbody2D;

		protected override void OnCreate()
        {
			Log.Info("Hello from bullet!");

			m_Rigidbody2D = GetComponent<Rigidbody2DComponent>();

			OnTriggerBegin2D += OnTriggerBegin;
		}

		protected override void OnUpdate(float ts)
        {
			m_Rigidbody2D.Velocity = Mathf.Lerp(m_Rigidbody2D.Velocity, Vector3.Zero, 3.0f * ts);

			if ((!m_OneTime && m_DeathTimer < 0.0f) || m_Destroy)
            {
                m_OneTime = true;
                Scene.DestroyEntity(this);
            }
            m_DeathTimer -= ts;
        }

		internal void SetDirection(Vector2 direction)
		{
			m_Rigidbody2D.Velocity = direction * Speed;

			float angle = Mathf.Atan(direction.Y / direction.X); // [-90,90]

			// Rotate respectively
			Vector3 rotation = Transform.Rotation;
			rotation.Z = angle;
			Transform.Rotation = rotation;
		}

		private void OnTriggerBegin(Entity other)
		{
			if (other.Name == "Hitbox-Trigger")
				return;

			m_Destroy = DestroyOnImpact;
		}
	}
}
