using System.Collections.Generic;
using Turbo;

namespace GunNRun
{
	internal enum PlayerAnimation
	{
		Idle = 0,
		Running,

		Count
	}

	internal class PlayerAnimator
	{
		private Player m_Player;
		private SpriteAnimator m_Animator;
		private Vector2 m_SpriteSize = new Vector2(20.0f, 20.0f);

		internal PlayerAnimator(Player player)
		{
			m_Player = player;
			m_Animator = new SpriteAnimator(m_Player.GetComponent<SpriteRendererComponent>(), (int)PlayerAnimation.Count);

			//Idle
			{
				var frameIndicies = new List<Vector2>(3)
				{
					new Vector2(0, 1),
					new Vector2(1, 1),
					new Vector2(2, 1),
				};

				m_Animator.AddAnimation(new SpriteAnimation((int)PlayerAnimation.Idle, frameIndicies, m_SpriteSize, m_Player.IdleAnimation, true));
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

				m_Animator.AddAnimation(new SpriteAnimation((int)PlayerAnimation.Running, frameIndicies, m_SpriteSize, m_Player.RunningAnimation, true));
			}

			ChangeAnimation(PlayerAnimation.Idle);
		}

		internal void OnUpdate()
		{
			if(m_Player.Velocity.Length != 0.0f)
			{
				ChangeAnimation(PlayerAnimation.Running);
			}
			else
			{
				ChangeAnimation(PlayerAnimation.Idle);
			}

			m_Animator.OnUpdate(Frame.TimeStep);
		}

		private void ChangeAnimation(PlayerAnimation animation)
		{
			m_Animator.ChangeAnimation((int)animation);
		}
	}
}
