using System;
using System.Collections.Generic;
using System.Text;
using Turbo;

namespace GunNRun
{
	public static class PlayerAnimation
	{
		public static readonly int Idle = 0;
		public static readonly int Running = 1;
		public static readonly int InAir = 2;
		public static readonly int ShootingIdle = 3;
		public static readonly int ShootingRunning = 4;
		public static readonly int ShootingInAir = 5;

		public static readonly int Count = 6;
	}

	internal class PlayerAnimator
	{
		private PlayerManager m_PlayerManager;
		private PlayerController m_PlayerController;
		private PlayerInput m_PlayerInput;

		private SpriteRendererComponent m_SpriteRenderer;
		private SpriteAnimator m_Animator;

		private readonly Vector2 m_SpriteSize = new Vector2(45, 45);

		internal void Init(PlayerManager playerManager)
		{
			m_PlayerManager = playerManager;
			m_PlayerController = m_PlayerManager.m_PlayerController;
			m_PlayerInput = m_PlayerManager.m_PlayerInput;

			m_SpriteRenderer = m_PlayerManager.GetComponent<SpriteRendererComponent>();

			m_Animator = new SpriteAnimator(m_SpriteRenderer, PlayerAnimation.Count);
			m_Animator.SetOnAnimationChangeCallback(OnAnimationChange);

			// Idle
			{
				var frameIndicies = new List<Vector2>(7)
				{
					new Vector2(0, -1),
					new Vector2(1, -1),
					new Vector2(2, -1),
					new Vector2(2, -1),
					new Vector2(3, -1),
					new Vector2(4, -1),
					new Vector2(5, -1)
				};

				m_Animator.AddAnimation(new SpriteAnimation(PlayerAnimation.Idle, frameIndicies, m_SpriteSize, m_PlayerManager.IdleAnimationDelay, true));
			}

			// Running
			{
				var frameIndicies = new List<Vector2>(8)
				{
					new Vector2(6, -1),
					new Vector2(7, -1),
					new Vector2(0, -2),
					new Vector2(1, -2),
					new Vector2(2, -2),
					new Vector2(3, -2),
					new Vector2(4, -2),
					new Vector2(5, -2)
				};

				m_Animator.AddAnimation(new SpriteAnimation(PlayerAnimation.Running, frameIndicies, m_SpriteSize, m_PlayerManager.RunningAnimationDelay, true));
			}

			// In air
			{
				var frameIndicies = new List<Vector2>(2)
				{
					new Vector2(7, -2),
					new Vector2(7, -2)
				};

				m_Animator.AddAnimation(new SpriteAnimation(PlayerAnimation.InAir, frameIndicies, m_SpriteSize, 0.006f, true));
			}

			// Idle shooting
			{
				var frameIndicies = new List<Vector2>(2)
				{
					new Vector2(2, -3),
					new Vector2(2, -3)
				};

				m_Animator.AddAnimation(new SpriteAnimation(PlayerAnimation.ShootingIdle, frameIndicies, m_SpriteSize, m_PlayerManager.IdleShootingAnimationDelay, true));
			}

			// Running shooting
			{
				var frameIndicies = new List<Vector2>(8)
				{
					new Vector2(0, -4),
					new Vector2(1, -4),
					new Vector2(2, -4),
					new Vector2(3, -4),
					new Vector2(4, -4),
					new Vector2(5, -4),
					new Vector2(6, -4),
					new Vector2(7, -4)
				};

				m_Animator.AddAnimation(new SpriteAnimation(PlayerAnimation.ShootingRunning, frameIndicies, m_SpriteSize, m_PlayerManager.RunShootingAnimationDelay, true));
			}

			// In air shooting
			{
				var frameIndicies = new List<Vector2>(2)
				{
					new Vector2(0, -6),
					new Vector2(1, -6)
				};

				m_Animator.AddAnimation(new SpriteAnimation(PlayerAnimation.ShootingInAir, frameIndicies, m_SpriteSize, m_PlayerManager.InAirShootingAnimationDelay, true));
			}

			m_Animator.ChangeAnimation(PlayerAnimation.Idle);
		}

		internal void OnUpdate(float ts)
		{
			Vector2 velocity = m_PlayerController.Velocity;


			var animIndex = m_Animator.GetCurrentAnimationID();
			if (velocity.X != 0.0f)
			{
				animIndex = PlayerAnimation.Running;
			}
			else
			{
				animIndex = PlayerAnimation.Idle;
			}

			if (m_PlayerInput.IsShootKeyPressedOneTime)
			{
				animIndex = velocity.X != 0.0f ? PlayerAnimation.ShootingRunning : PlayerAnimation.ShootingIdle;
			}

			if (m_PlayerController.IsInAir)
			{
				animIndex = m_PlayerInput.IsShootKeyPressedOneTime ? PlayerAnimation.ShootingInAir : PlayerAnimation.InAir;
			}

			m_Animator.ChangeAnimation(animIndex);

			m_Animator.OnUpdate(ts);
		}

		private void OnAnimationChange(SpriteAnimation current, ref SpriteAnimation next)
		{
			// Idle <=> Running
			if ((current.ID == PlayerAnimation.Running && next.ID == PlayerAnimation.ShootingRunning) ||
				(current.ID == PlayerAnimation.ShootingRunning && next.ID == PlayerAnimation.Running))
			{
				next.CurrentIndex = current.CurrentIndex;
				next.CurrentTime = current.CurrentTime;
			}
		}
	}
}
