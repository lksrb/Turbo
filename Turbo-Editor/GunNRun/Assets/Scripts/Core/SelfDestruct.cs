using Turbo;

namespace GunNRun
{
	// Utility script for disposable entities
	// We use this especially in audio sources
	public class SelfDestruct : Entity
	{
		public readonly float TimeTillDestruction;

		private SingleTickTimer m_Timer;

		protected override void OnCreate()
		{
			m_Timer = new SingleTickTimer(TimeTillDestruction);
		}

		protected override void OnUpdate()
		{
			if(m_Timer)
			{
				Scene.DestroyEntity(this);
			}
		}
	}
}
