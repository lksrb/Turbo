using Turbo;

namespace GunNRun
{
	public class Enemy : Entity
	{
		private Rigidbody2DComponent m_Rigidbody2D;
		private Player m_Player;

		protected override void OnCreate()
		{
			m_Rigidbody2D = GetComponent<Rigidbody2DComponent>();
			m_Player = FindEntityByName("Player").As<Player>();
		}

		protected override void OnUpdate(float ts)
		{

		}
	}
}
