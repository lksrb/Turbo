using Turbo;

namespace GunNRun
{
	public class Timer
	{
		private float Current;
		private readonly float Duration;
		private readonly bool AutoReset;

		public Timer(float duration, bool autoReset = true)
		{
			Current = 0.0f;
			Duration = duration;
			AutoReset = autoReset;
		}

		public static implicit operator bool(Timer timer)
		{
			timer.Current += Frame.TimeStep;

			if (timer.Current > timer.Duration)
			{
				if (timer.AutoReset)
					timer.Reset();
				return true;
			}

			return false;
		}

		public float Delta => Duration - Current;

		public void Reset()
		{
			Current = 0.0f;
		}
	}

}
