using Turbo;

namespace GunNRun
{
	internal enum EnemyAnimation : uint
	{
		Idle = 0,
		Running,
		InAir,
		ShootingIdle,
		ShootingRunning,
	}

	internal class EnemyAnimator
	{
		private Enemy m_Enemy;

		private Animation m_CurrentAnimation;
		private SpriteRendererComponent m_SpriteRenderer;
		private Animation[] m_Animations;
		private readonly Vector2 m_SpriteSize = new Vector2(32, 38);

		internal void Init(Enemy enemy)
		{
			m_Enemy = enemy;

			m_SpriteRenderer = m_Enemy.GetComponent<SpriteRendererComponent>();

			m_Animations = new Animation[10];
			// Idle
			{
				var frameIndicies = new FixedArray<Vector2>(7);
				frameIndicies.PushBack(new Vector2(0, 0));
				frameIndicies.PushBack(new Vector2(1, 0));
				frameIndicies.PushBack(new Vector2(2, 0));

				m_Animations[(uint)PlayerAnimation.Idle] = new Animation(PlayerAnimation.Idle, true, m_Enemy.m_EnemyManager.IdleAnimationSpeed, frameIndicies);
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

				m_Animations[(uint)PlayerAnimation.Running] = new Animation(PlayerAnimation.Running, true, 0.004f, frameIndicies);
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
/*
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

			ChangeAnimation(animIndex);*/
		}

		private void OnUpdateAnimation(float ts)
		{
			m_CurrentAnimation.AnimationSpeed = m_Enemy.m_EnemyManager.IdleAnimationSpeed;

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
