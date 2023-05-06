using Turbo;
using System.Collections.Generic;
using System;

namespace GunNRun
{
	public class SpriteAnimator
	{
		public delegate void OnAnimationChangeCallback(SpriteAnimation current, ref SpriteAnimation next);

		private List<SpriteAnimation> m_Animations;
		private SpriteAnimation m_CurrentAnimation = SpriteAnimation.Invalid;
		private SpriteRendererComponent m_SpriteRenderer;
		private OnAnimationChangeCallback m_OnAnimationChangeCallback;

		public SpriteAnimator(SpriteRendererComponent spriteRenderer, int animationCount = 1)
		{
			m_SpriteRenderer = spriteRenderer;
			m_Animations = new List<SpriteAnimation>(animationCount);
		}

		public void SetOnAnimationChangeCallback(OnAnimationChangeCallback onAnimChangeAction)
		{
			m_OnAnimationChangeCallback = onAnimChangeAction;
		}

		public void AddAnimation(SpriteAnimation animation)
		{
			m_Animations.Add(animation);
		}

		public void ChangeAnimation(int id)
		{
			if (m_CurrentAnimation.ID == id)
				return;

			SpriteAnimation next = GetAnimation(id);

			if (next == SpriteAnimation.Invalid)
			{
				Log.Error($"Couldn't find the animation! ({id})");
				return;
			}

			m_OnAnimationChangeCallback?.Invoke(m_CurrentAnimation, ref next);

			m_CurrentAnimation.Reset();
			
			m_CurrentAnimation = next;
		}

		public void OnUpdate(float ts)
		{
			if (m_CurrentAnimation == SpriteAnimation.Invalid)
				return;

			var animationSpritePosition = m_CurrentAnimation.CurrentFrame(ts);
			m_SpriteRenderer.SetSpriteBounds(animationSpritePosition, m_CurrentAnimation.SpriteSize);
		}

		public int GetCurrentAnimationID() => m_CurrentAnimation.ID;

		private SpriteAnimation GetAnimation(int id)
		{
			foreach (var animation in m_Animations)
			{
				if (animation.ID == id)
					return animation;
			}

			return SpriteAnimation.Invalid;
		}
	}
}
