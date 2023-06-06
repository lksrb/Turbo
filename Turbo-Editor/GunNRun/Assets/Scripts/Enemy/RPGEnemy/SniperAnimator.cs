using Turbo;
using System.Collections.Generic;

namespace GunNRun
{
	internal class SniperAnimator
	{
		private static class SniperAnimation
		{
			public static readonly int Idle = 0;
			public static readonly int Count = 1;
		}

		private SniperEnemy m_SniperEnemy;
		private SpriteAnimator m_Animator;
		private Vector2 m_SpriteSize = new Vector2(20.0f, 20.0f);

		internal SniperAnimator(SniperEnemy enemy)
		{
			m_SniperEnemy = enemy;
			m_Animator = new SpriteAnimator(m_SniperEnemy.GetComponent<SpriteRendererComponent>(), SniperAnimation.Count);

			//Idle
			{
				var frameIndicies = new List<Vector2>(3)
				{
					new Vector2(0, 1),
					new Vector2(1, 1),
					new Vector2(2, 1),
				};

				m_Animator.AddAnimation(new SpriteAnimation(SniperAnimation.Idle, frameIndicies, m_SpriteSize, enemy.IdleAnimation, true));
			}

			m_Animator.ChangeAnimation(SniperAnimation.Idle);
		}

		internal void OnUpdate()
		{
			m_Animator.OnUpdate(Frame.TimeStep);
		}
	}
}
