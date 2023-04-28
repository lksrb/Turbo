using System;
using System.Text;
using Turbo;

namespace GunNRun
{
	internal enum PlayerAnimation : uint
	{
		Idle = 0,
		Running,
		InAir,
		ShootingIdle,
		ShootingRunning,
		ShootingInAir
	}

	internal struct Animation
	{
		internal PlayerAnimation Index;
		internal float AnimationSpeed;
		internal uint AnimationIndex;
		internal bool Repeat;
		internal float CurrentTime;

		private FixedArray<Vector2> AnimationFrames;

		internal Animation(PlayerAnimation index, bool repeat, float speed, FixedArray<Vector2> frames)
		{
			Index = index;
			Repeat = repeat;
			AnimationSpeed = speed;
			AnimationFrames = frames;
			AnimationIndex = 0;
			CurrentTime = 0;
		}

		internal void Reset()
		{
			AnimationIndex = 0;
			CurrentTime = 0.0f;
		}

		internal void NextFrame()
		{
			AnimationIndex = (AnimationIndex + 1) % AnimationFrames.Length;
		}

		internal Vector2 CurrentFrame(float ts)
		{
			CurrentTime += ts;

			Vector2 currentAnimationFrame = AnimationFrames[AnimationIndex];
			if (CurrentTime > AnimationSpeed)
			{
				if (Repeat)
					CurrentTime = 0.0f;

				NextFrame();
			}

			return currentAnimationFrame;
		}

		public Animation Clone()
		{
			return new Animation(Index, Repeat, AnimationSpeed, AnimationFrames);
		}
	}

	internal class PlayerAnimator
	{
		private PlayerManager m_PlayerManager;
		private PlayerController m_PlayerController;
		private PlayerInput m_PlayerInput;

		private Animation m_CurrentAnimation;
		private SpriteRendererComponent m_SpriteRenderer;
		private Animation[] m_Animations;
		private readonly Vector2 m_SpriteSize = new Vector2(45, 45);

		internal void Init(PlayerManager playerManager)
		{
			m_PlayerManager = playerManager;
			m_PlayerController = m_PlayerManager.m_PlayerController;
			m_PlayerInput = m_PlayerManager.m_PlayerInput;

			m_SpriteRenderer = m_PlayerManager.GetComponent<SpriteRendererComponent>();

			m_Animations = new Animation[10];
			// Idle
			{
				var frameIndicies = new FixedArray<Vector2>(7);
				frameIndicies.PushBack(new Vector2(0, -1));
				frameIndicies.PushBack(new Vector2(1, -1));
				frameIndicies.PushBack(new Vector2(2, -1));
				frameIndicies.PushBack(new Vector2(2, -1));
				frameIndicies.PushBack(new Vector2(3, -1));
				frameIndicies.PushBack(new Vector2(4, -1));
				frameIndicies.PushBack(new Vector2(5, -1));

				m_Animations[(uint)PlayerAnimation.Idle] = new Animation(PlayerAnimation.Idle, true, m_PlayerManager.m_AnimationIdleSpeed, frameIndicies);
			}

			// Running
			{
				var frameIndicies = new FixedArray<Vector2>(8);
				frameIndicies.PushBack(new Vector2(6, -1));
				frameIndicies.PushBack(new Vector2(7, -1));
				frameIndicies.PushBack(new Vector2(0, -2));
				frameIndicies.PushBack(new Vector2(1, -2));
				frameIndicies.PushBack(new Vector2(2, -2));
				frameIndicies.PushBack(new Vector2(3, -2));
				frameIndicies.PushBack(new Vector2(4, -2));
				frameIndicies.PushBack(new Vector2(5, -2));

				m_Animations[(uint)PlayerAnimation.Running] = new Animation(PlayerAnimation.Running, true, 0.006f, frameIndicies);
			}

			// In air
			{
				var frameIndicies = new FixedArray<Vector2>(2);
				frameIndicies.PushBack(new Vector2(7, -2));
				frameIndicies.PushBack(new Vector2(7, -2));

				m_Animations[(uint)PlayerAnimation.InAir] = new Animation(PlayerAnimation.InAir, true, 0.006f, frameIndicies);
			}

			// Idle shooting
			{
				var frameIndicies = new FixedArray<Vector2>(2);
				frameIndicies.PushBack(new Vector2(2, -3));
				frameIndicies.PushBack(new Vector2(3, -3));

				m_Animations[(uint)PlayerAnimation.ShootingIdle] = new Animation(PlayerAnimation.ShootingIdle, true, 0.006f, frameIndicies);
			}

			// Running shooting
			{
				var frameIndicies = new FixedArray<Vector2>(8);
				frameIndicies.PushBack(new Vector2(0, -4));
				frameIndicies.PushBack(new Vector2(1, -4));
				frameIndicies.PushBack(new Vector2(2, -4));
				frameIndicies.PushBack(new Vector2(3, -4));
				frameIndicies.PushBack(new Vector2(4, -4));
				frameIndicies.PushBack(new Vector2(5, -4));
				frameIndicies.PushBack(new Vector2(6, -4));
				frameIndicies.PushBack(new Vector2(7, -4));

				m_Animations[(uint)PlayerAnimation.ShootingRunning] = new Animation(PlayerAnimation.ShootingRunning, true, 0.006f, frameIndicies);
			}

			// In air shooting
			{
				var frameIndicies = new FixedArray<Vector2>(2);
				frameIndicies.PushBack(new Vector2(0, -6));
				frameIndicies.PushBack(new Vector2(1, -6));

				m_Animations[(uint)PlayerAnimation.ShootingInAir] = new Animation(PlayerAnimation.ShootingInAir, true, 0.002f, frameIndicies);
			}

			// Set current animation
			m_CurrentAnimation = GetAnimation(PlayerAnimation.Idle);
		}

		internal void OnUpdate(float ts)
		{
			OnUpdateAnimation(ts);

			var animIndex = m_CurrentAnimation.Index;
			if (m_PlayerController.IsMovingSideways)
			{
				animIndex = PlayerAnimation.Running;
			}
			else
			{
				animIndex = PlayerAnimation.Idle;
			}

			if (m_PlayerInput.IsShootKeyPressedOneTime)
			{
				animIndex = m_PlayerController.IsMovingSideways ? PlayerAnimation.ShootingRunning : PlayerAnimation.ShootingIdle;
			}

			if (m_PlayerController.IsInAir)
			{
				animIndex = m_PlayerInput.IsShootKeyPressedOneTime ? PlayerAnimation.ShootingInAir : PlayerAnimation.InAir;
			}

			ChangeAnimation(animIndex);
		}

		private void OnUpdateAnimation(float ts)
		{
			m_CurrentAnimation.AnimationSpeed = m_PlayerManager.m_AnimationIdleSpeed;

			var animationSpritePosition = m_CurrentAnimation.CurrentFrame(ts);
			m_SpriteRenderer.SetSpriteBounds(animationSpritePosition, m_SpriteSize);
		}

		private void ChangeAnimation(PlayerAnimation animation)
		{
			if (m_CurrentAnimation.Index == animation)
				return;

			Animation nextAnimation = GetAnimation(animation);
			// From idle to running
			if ((m_CurrentAnimation.Index == PlayerAnimation.Running && animation == PlayerAnimation.ShootingRunning) ||
				(m_CurrentAnimation.Index == PlayerAnimation.ShootingRunning && animation == PlayerAnimation.Running))
			{
				nextAnimation.AnimationIndex = m_CurrentAnimation.AnimationIndex;
				nextAnimation.CurrentTime = m_CurrentAnimation.CurrentTime;
				//nextAnimation.NextFrame();
			}

			Log.Info($"{m_CurrentAnimation.Index} -> {nextAnimation.Index}");

			m_CurrentAnimation = nextAnimation;
		}

		private Animation GetAnimation(PlayerAnimation animation)
		{
			return m_Animations[(uint)animation].Clone();
		}
	}
}
