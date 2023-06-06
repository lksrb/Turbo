using Turbo;
using System.Collections.Generic;

namespace GunNRun
{
	internal class SuiciderAnimator
	{
		private static class SuiciderAnimation
		{
			public static readonly int Running = 0;
			public static readonly int Count = 1;
		}

		private SuiciderEnemy m_SuiciderEnemy;
		private SpriteAnimator m_Animator;
		private Vector2 m_SpriteSize = new Vector2(20.0f, 20.0f);

		internal SuiciderAnimator(SuiciderEnemy suiciderEnemy)
		{
			m_SuiciderEnemy = suiciderEnemy;
			m_Animator = new SpriteAnimator(m_SuiciderEnemy.GetComponent<SpriteRendererComponent>(), SuiciderAnimation.Count);

			// Running
			{
				var frameIndicies = new List<Vector2>(4)
				{
					new Vector2(0, 0),
					new Vector2(1, 0),
					new Vector2(2, 0),
					new Vector2(3, 0)
				};

				m_Animator.AddAnimation(new SpriteAnimation(SuiciderAnimation.Running, frameIndicies, m_SpriteSize, m_SuiciderEnemy.RunningAnimation, true));
			}

			m_Animator.ChangeAnimation(SuiciderAnimation.Running);
		}

		internal void OnUpdate()
		{
			m_Animator.OnUpdate(Frame.TimeStep);
		}
	}
}
