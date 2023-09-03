using Turbo;

namespace Mystery
{
	public class Timer
	{
		private float Current;
		private bool Started;
		private readonly bool AutoStart;
		private readonly float Duration;
		private readonly bool AutoReset;

		public Timer(float duration, bool autoReset = true, bool autoStart = true)
		{
			Current = 0.0f;
			Duration = duration;
			AutoReset = autoReset;
			Started = autoStart;
			AutoStart = autoStart;
		}

		public static implicit operator bool(Timer timer)
		{
			if (!timer.Started)
				return false;

			timer.Current += Frame.TimeStep;

			if (timer.Current > timer.Duration)
			{
				if (timer.AutoReset)
				{
					timer.Reset();
				}
				return true;
			}

			return false;
		}

		public float Delta => Duration - Current;
		public bool IsStarted => IsStarted;

		public void Start()
		{
			Started = true;
		}

		public void Stop()
		{
			Started = false;
		}

		public void Reset()
		{
			if (!AutoStart)
				Started = false;

			Current = 0.0f;
		}
	}
}
