using Turbo;

namespace GunNRun
{
	public class Bullet : Entity
	{
		public readonly bool DestroyOnImpact = false;

		private static readonly string s_BulletPrefab = "Assets/Prefabs/Bullet.tprefab";

		public Entity ShooterEntity { get; private set; }

		private Timer m_DeathTimer;

		private float m_SpeedDecentFactor = 1.0f; 
		private bool m_Destroy = false;

		private Rigidbody2DComponent m_Rigidbody2D;
		private bool m_Init = false;

		protected override void OnCreate()
		{
			m_Rigidbody2D = GetComponent<Rigidbody2DComponent>();

			CollisionFilter filter = new CollisionFilter();
			filter.CollisionCategory = (ushort)EntityCategory.Bullet;
			filter.CollisionMask = (ushort)EntityCategory.Player | (ushort)EntityCategory.Enemy | (ushort)EntityCategory.Wall;
			GetComponent<BoxCollider2DComponent>().Filter = filter;

			OnTriggerBegin2D += OnDestroy;
		}

		protected override void OnUpdate()
		{
			if (!m_Init)
			{
				Log.Error("Bullet is not initialized!");
				return;
			}

			m_Rigidbody2D.Velocity = Mathf.Lerp(m_Rigidbody2D.Velocity, Vector3.Zero, m_SpeedDecentFactor * Frame.TimeStep);

			if (m_DeathTimer || m_Destroy)
			{
				Scene.DestroyEntity(this);
			}
		}
		
		internal static Bullet Create(Entity shooter, Vector3 translation, Vector2 direction, float speed = 20.0f, float lifeSpan = 1.0f, float speedDecentFactor = 1.0f)
		{
			Bullet bullet = shooter.InstantiateChild(s_BulletPrefab, translation).As<Bullet>();

			bullet.ShooterEntity = shooter;

			bullet.m_Rigidbody2D.Velocity = direction * speed * Random.Float(1.0f, 1.5f);

			// Rotate respectively
			float angle = Mathf.Atan(direction.Y / direction.X); // [-90,90]
			Vector3 rotation = bullet.Transform.Rotation;
			rotation.Z = angle;
			bullet.Transform.Rotation = rotation;

			bullet.m_SpeedDecentFactor = speedDecentFactor;
			bullet.m_DeathTimer = new Timer(lifeSpan, false);

			bullet.m_Init = true;

			Log.Warn("Bullet initialized!");

			return bullet;
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
