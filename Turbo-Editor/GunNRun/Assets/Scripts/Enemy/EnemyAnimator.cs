
using System.Collections.Generic;
using Turbo;

namespace GunNRun
{
	public static class EnemyAnimation
	{
		public static readonly int Idle = 0;
		public static readonly int Running = 1;
		public static readonly int ShootingIdle = 2;
		public static readonly int ShootingRunning = 3;
		public static readonly int Dying = 4;

		public static readonly int Count = 5;
	}

	internal class EnemyAnimator
	{
		private Enemy m_Enemy;

		private SpriteRendererComponent m_SpriteRenderer;
		private SpriteAnimator m_Animator;

		private readonly Vector2 m_SpriteSize = new Vector2(32, 38);

		internal void Init(Enemy enemy)
		{
			m_Enemy = enemy;

			m_SpriteRenderer = m_Enemy.GetComponent<SpriteRendererComponent>();
			m_Animator = new SpriteAnimator(m_SpriteRenderer, EnemyAnimation.Count);
			m_Animator.SetOnAnimationChangeCallback(OnAnimationChange);

			// Idle
			{
				var frameIndicies = new List<Vector2>(3)
				{
					new Vector2(0, 0),
					new Vector2(1, 0),
					new Vector2(2, 0)
				};

				m_Animator.AddAnimation(new SpriteAnimation(EnemyAnimation.Idle, frameIndicies, m_SpriteSize,  m_Enemy.m_EnemyManager.IdleAnimationDelay, true));
			}

			// Running
			{
				var frameIndicies = new List<Vector2>(8)
				{
					new Vector2(11, 0),
					new Vector2(12, 0),
					new Vector2(13, 0),
					new Vector2(14, 0),
					new Vector2(15, 0),
					new Vector2(16, 0),
					new Vector2(17, 0),
					new Vector2(18, 0)
				};

				m_Animator.AddAnimation(new SpriteAnimation(EnemyAnimation.Running, frameIndicies, m_SpriteSize, m_Enemy.m_EnemyManager.RunningAnimationDelay, true));
			}

			// Idle shooting
			{
				var frameIndicies = new List<Vector2>(2)
				{
					new Vector2(2, -3),
					new Vector2(3, -3)
				};
				m_Animator.AddAnimation(new SpriteAnimation(EnemyAnimation.ShootingIdle, frameIndicies, m_SpriteSize, m_Enemy.m_EnemyManager.IdleShootingAnimationDelay, true));
			}

			// Running shooting
			{
				var frameIndicies = new List<Vector2>(8)
				{
					new Vector2(0, 0),
					new Vector2(1, 0),
					new Vector2(2, 0),
					new Vector2(3, 0),
					new Vector2(4, 0),
					new Vector2(5, 0),
					new Vector2(6, 0),
					new Vector2(7, 0)
				};

				m_Animator.AddAnimation(new SpriteAnimation(EnemyAnimation.ShootingRunning, frameIndicies, m_SpriteSize, m_Enemy.m_EnemyManager.RunShootingAnimationDelay, true));
			}

			// Dying
			{
				var frameIndicies = new List<Vector2>(4)
				{
					new Vector2(19, 0),
					new Vector2(20, 0),
					new Vector2(21, 0),
					new Vector2(22, 0)
				};

				m_Animator.AddAnimation(new SpriteAnimation(EnemyAnimation.Dying, frameIndicies, m_SpriteSize, 0.150f, false));
			}

			m_Animator.ChangeAnimation(EnemyAnimation.Idle);
		}

		internal void OnUpdate(float ts)
		{
			Vector2 velocity = m_Enemy.Velocity;

			var animIndex = m_Animator.GetCurrentAnimationID();
			if (velocity.X != 0.0f)
			{
				animIndex = EnemyAnimation.Running;
			}
			else
			{
				animIndex = EnemyAnimation.Idle;
			}

			if(m_Enemy.m_Health <= 0)
			{
				animIndex = EnemyAnimation.Dying;
			}

		/*	if (m_PlayerInput.IsShootKeyPressedOneTime)
			{
				animIndex = velocity.X != 0.0f ? PlayerAnimation.ShootingRunning : PlayerAnimation.ShootingIdle;
			}
*/

			m_Animator.ChangeAnimation(animIndex);
			m_Animator.OnUpdate(ts);
		}

		private void OnAnimationChange(SpriteAnimation current, ref SpriteAnimation next)
		{
			// Idle <=> Running
			if ((current.ID == EnemyAnimation.Running && next.ID == EnemyAnimation.ShootingRunning) ||
				(current.ID == EnemyAnimation.ShootingRunning && next.ID == EnemyAnimation.Running))
			{
				next.CurrentIndex = current.CurrentIndex;
				next.CurrentTime = current.CurrentTime;
			}
		}
	}
}

