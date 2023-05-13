using Turbo;

namespace GunNRun
{
	public class Timer
	{
		private float Current;
		private readonly float Max;
		private readonly bool AutoReset;

		public Timer(float max, bool autoReset = true)
		{
			Current = 0.0f;
			Max = max;
			AutoReset = autoReset;
		}

		public static implicit operator bool(Timer timer)
		{
			timer.Current += Frame.TimeStep;

			if (timer.Current > timer.Max)
			{
				if (timer.AutoReset)
					timer.Reset();
				return true;
			}

			return false;
		}

		public float Delta => Max - Current;

		public void Reset()
		{
			Current = 0.0f;
		}
	}

}
