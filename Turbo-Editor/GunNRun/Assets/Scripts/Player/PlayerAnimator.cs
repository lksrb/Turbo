using System.Collections.Generic;
using Turbo;

namespace GunNRun
{
	internal class PlayerAnimator
	{
		private static class PlayerAnimation
		{
			public static readonly int Idle = 0;
			public static readonly int Running = 1;
			public static readonly int Count = 2;
		}

		private Player m_Player;
		private SpriteAnimator m_Animator;
		private Vector2 m_SpriteSize = new Vector2(16.0f, 16.0f);

		internal void Init(Player player)
		{
			m_Player = player;
			m_Animator = new SpriteAnimator(m_Player.SpriteRenderer, PlayerAnimation.Count);

			//Idle
			{
				var frameIndicies = new List<Vector2>(3)
				{
					new Vector2(0, 1),
					new Vector2(1, 1),
					new Vector2(2, 1),
				};

				m_Animator.AddAnimation(new SpriteAnimation(PlayerAnimation.Idle, frameIndicies, m_SpriteSize, m_Player.IdleAnimation, true));
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

				m_Animator.AddAnimation(new SpriteAnimation(PlayerAnimation.Running, frameIndicies, m_SpriteSize, m_Player.RunningAnimation, true));
			}

			m_Animator.ChangeAnimation(PlayerAnimation.Idle);
		}

		internal void OnUpdate(float ts)
		{
			

			if(m_Player.Velocity.Length != 0.0f)
			{
				m_Animator.ChangeAnimation(PlayerAnimation.Running);
			}
			else
			{
				m_Animator.ChangeAnimation(PlayerAnimation.Idle);
			}

			m_Animator.OnUpdate(ts);
		}
	}
}
