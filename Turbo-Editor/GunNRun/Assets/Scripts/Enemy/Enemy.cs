using System;
using System.Runtime.InteropServices;
using Turbo;

namespace GunNRun
{
	public class Enemy : Entity
	{
		private Rigidbody2DComponent m_Rigidbody2D;
		private EnemyAnimator m_Animator = new EnemyAnimator();
		private float m_Speed = 5.0f;
		internal int m_Health = 5;

		internal Vector2 Velocity { get; private set; } = Vector2.Zero;

		internal EnemyManager m_EnemyManager;

		private Random m_Random = new Random();
		private float m_NextMoveCurrentTime = 2.0f;

		protected override void OnCreate()
		{
			m_EnemyManager = FindEntityByName("EnemyManager").As<EnemyManager>();
			m_Rigidbody2D = GetComponent<Rigidbody2DComponent>();

			m_Animator.Init(this);

			OnCollisionBegin2D += OnCollision;
			OnTriggerBegin2D += OnTriggerBegin;

			Log.Info("Enemy spawned!");
		}

		protected override void OnUpdate(float ts)
		{
			Vector2 velocity = m_Rigidbody2D.Velocity;

			if (m_NextMoveCurrentTime > 2.0f)
			{
				m_NextMoveCurrentTime = 0.0f;

				int lol = m_Random.Next(0, 5);

				//Log.Info(lol);

				/*if (lol == 1)
				{
					m_Speed = 5.0f;
				} else
				{
					m_Speed = 0.0f;
				}*/
			}
			velocity.X = m_Speed;

			m_NextMoveCurrentTime += ts;

			Velocity = velocity;

			if(m_Health <= 0)
			{
				velocity.X = 0;
				m_Rigidbody2D.IsEnabled = false;
			}

			m_Rigidbody2D.Velocity = velocity;

			if (Velocity.X != 0)
			{
				var scale = Transform.Scale;
				var velocityX = Velocity.X;

				Vector3 rotatedScale = new Vector3(Mathf.Sign(velocityX) * Mathf.Abs(scale.X), scale.Y, scale.Z);
				Transform.Scale = rotatedScale;
			}

			m_Animator.OnUpdate(ts);
		}
		
		private void OnCollision(Entity other)
		{
			switch (other.Name)
			{
				case "Hitbox-Horizontal":
					m_Speed *= -1;
					break;
				case "Bullet":
					Log.Info("HIT!");

					--m_Health;
					break;
			}
		}

		private void OnTriggerBegin(Entity other)
		{
			switch (other.Name)
			{
				case "Hitbox-Trigger":
					m_Speed *= -1;
					break;
			}
		}
	}
}
