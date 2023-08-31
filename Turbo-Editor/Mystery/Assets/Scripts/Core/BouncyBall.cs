using Turbo;

namespace Mystery
{
	public class BouncyBall : Entity
	{
		private Entity m_Holder;

		internal bool SetHolder(Entity entity)
		{
			if(m_Holder == null)
			{
				m_Holder = entity;
				return true;
			}

			return false;
		}
		
		internal void ReleaseFromHolder(Entity entity)
		{
			if(m_Holder == entity)
			{
				m_Holder = null;
			}
		}

		protected override void OnCreate()
		{
		}

		protected override void OnUpdate()
		{
		}
	}
}
