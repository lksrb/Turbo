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

	public class SingleUseTimer
	{
		private Timer m_Timer;
		private bool m_Once;

		public SingleUseTimer(float duration)
		{
			m_Timer = new Timer(duration);
			m_Once = false;
		}

		public static implicit operator bool(SingleUseTimer timer)
		{
			if(!timer.m_Once && timer.m_Timer)
			{
				timer.m_Once = true;

				return true;
			}

			return false;
		}

		public void Reset()
		{
			m_Once = false;
			m_Timer.Reset();
		}

		public float Delta => m_Timer.Delta;

	}
}
