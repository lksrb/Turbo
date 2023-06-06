using Turbo;
using System.Collections.Generic;

namespace GunNRun
{
	internal class ShooterAnimator
	{
		private static class ShooterAnimation
		{
			public static readonly int Idle = 0;
			public static readonly int Running = 1;
			public static readonly int Count = 2;
		}

		private ShooterEnemy m_ShooterEnemy;
		private SpriteAnimator m_Animator;
		private Vector2 m_SpriteSize = new Vector2(20.0f, 20.0f);

		internal ShooterAnimator(ShooterEnemy shooterEnemy)
		{
			m_ShooterEnemy = shooterEnemy;
			m_Animator = new SpriteAnimator(m_ShooterEnemy.GetComponent<SpriteRendererComponent>(), ShooterAnimation.Count);

			//Idle
			{
				var frameIndicies = new List<Vector2>(3)
				{
					new Vector2(0, 1),
					new Vector2(1, 1),
					new Vector2(2, 1),
				};

				m_Animator.AddAnimation(new SpriteAnimation(ShooterAnimation.Idle, frameIndicies, m_SpriteSize, m_ShooterEnemy.IdleAnimation, true));
			}

			// Running
			{
				var frameIndicies = new List<Vector2>(4)
				{
					new Vector2(0, 0),
					new Vector2(1, 0),
					new Vector2(2, 0),
					new Vector2(3, 0)
				};

				m_Animator.AddAnimation(new SpriteAnimation(ShooterAnimation.Running, frameIndicies, m_SpriteSize, m_ShooterEnemy.RunningAnimation, true));
			}

			m_Animator.ChangeAnimation(ShooterAnimation.Idle);
		}

		internal void OnUpdate()
		{
			if (m_ShooterEnemy.Velocity.Length != 0.0f)
			{
				m_Animator.ChangeAnimation(ShooterAnimation.Running);
			}
			else
			{
				m_Animator.ChangeAnimation(ShooterAnimation.Idle);
			}

			m_Animator.OnUpdate(Frame.TimeStep);
		}
	}
}
