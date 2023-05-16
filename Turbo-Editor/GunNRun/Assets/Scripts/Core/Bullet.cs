using Turbo;

namespace GunNRun
{
	public class Bullet : Entity
	{
		public readonly bool DestroyOnImpact = false;

		public Entity ShooterEntity { get; private set; }

		private Timer m_DeathTimer = new Timer(1.0f, false);

		private float m_Speed = 0.0f;
		private bool m_Destroy = false;

		private Rigidbody2DComponent m_Rigidbody2D;
		private bool m_Init = false;

		protected override void OnCreate()
		{
			m_Rigidbody2D = GetComponent<Rigidbody2DComponent>();

			CollisionFilter filter = new CollisionFilter();
			filter.CollisionCategory = (ushort)GameCategory.Bullet;
			filter.CollisionMask = (ushort)GameCategory.Player | (ushort)GameCategory.Enemy | (ushort)GameCategory.Wall;
			GetComponent<BoxCollider2DComponent>().Filter = filter;

			OnCollisionBegin2D += OnDestroy;

			Log.Info("Hello from bullet!");
		}

		protected override void OnUpdate()
		{
			if (!m_Init)
			{
				Log.Error("Bullet is not initialized!");
				return;
			}

			m_Rigidbody2D.Velocity = Mathf.Lerp(m_Rigidbody2D.Velocity, Vector3.Zero, 3.0f * Frame.TimeStep);

			if (m_DeathTimer || m_Destroy)
			{
				Scene.DestroyEntity(this);
			}
		}

		internal void Init(Entity shooter, Vector2 direction, float speed = 20.0f)
		{
			if (m_Init)
			{
				Log.Error("Bullet is already initialized!");
				return;
			}

			ShooterEntity = shooter;
			m_Speed = speed;

			m_Rigidbody2D.Velocity = direction * m_Speed * Random.Float(1.0f, 1.5f);

			// Rotate respectively
			float angle = Mathf.Atan(direction.Y / direction.X); // [-90,90]
			Vector3 rotation = Transform.Rotation;
			rotation.Z = angle;
			Transform.Rotation = rotation;

			m_Init = true;
		}

		private void OnDestroy(Entity other)
		{
			if (!m_Init)
			{
				Log.Error("Bullet is not initialized!");
				return;
			}

			Log.Info(other.Name);

			if (ShooterEntity == other || other.Name == "Bullet")
				return;

			m_Destroy = DestroyOnImpact;
		}
	}
}
