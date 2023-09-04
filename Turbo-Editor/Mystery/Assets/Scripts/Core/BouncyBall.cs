using Turbo;

namespace Mystery
{
	public class BouncyBall : Entity
	{
		private Entity m_Owner;

		internal bool SetOwner(Entity entity)
		{
			if (m_Owner == null)
			{
				m_Owner = entity;
				return true;
			}

			return false;
		}

		internal void Release(Entity entity)
		{
			if (m_Owner == entity)
				m_Owner = null;
		}

		internal Entity Owner => m_Owner;

		protected override void OnCreate()
		{
		}

		protected override void OnUpdate()
		{
		}
	}
}
