using System.Collections.Generic;
using System.Runtime.Serialization;
using Turbo;

namespace GunNRun
{
	public struct SpriteAnimation
	{
		public float AnimationDelay;
		public readonly Vector2 SpriteSize;
		public readonly int ID;
		public long CurrentIndex;
		public float CurrentTime;

		private readonly bool Repeat;
		private readonly List<Vector2> AnimationFrames;

		// Invalid state
		private SpriteAnimation(int id)
		{
			ID = id;
			AnimationFrames = new List<Vector2>();
			AnimationDelay = 0;
			Repeat = false;
			SpriteSize = Vector2.Zero;
			CurrentIndex = 0;
			CurrentTime = 0;
		}

		public SpriteAnimation(int id, List<Vector2> frames, Vector2 spriteSize, float delay, bool repeat)
		{
			ID = id;
			AnimationFrames = frames;
			AnimationDelay = delay;
			Repeat = repeat;
			SpriteSize = spriteSize;
			CurrentIndex = 0;
			CurrentTime = 0;
		}

		public static SpriteAnimation Invalid => new SpriteAnimation(-1);

		public static bool operator ==(SpriteAnimation a, SpriteAnimation b) => a.ID == b.ID;
		public static bool operator !=(SpriteAnimation a, SpriteAnimation b) => !(a == b);

		public void Reset()
		{
			CurrentIndex = 0;
			CurrentTime = 0.0f;
		}

		public void NextFrame()
		{
			if(CurrentIndex == AnimationFrames.Count - 1 && !Repeat)
				return;

			CurrentIndex = (CurrentIndex + 1) % AnimationFrames.Count;
		}

		public Vector2 CurrentFrame(float ts)
		{
			CurrentTime += ts;

			Vector2 currentAnimationFrame = AnimationFrames[(int)CurrentIndex];
			if (CurrentTime > AnimationDelay)
			{
				CurrentTime = 0.0f;
				NextFrame();
			}

			return currentAnimationFrame;
		}

		public override bool Equals(object obj) => base.Equals(obj);
		public override int GetHashCode() => base.GetHashCode();
	}

}
