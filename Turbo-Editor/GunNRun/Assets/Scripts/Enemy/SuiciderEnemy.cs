using System;
using Turbo;

namespace GunNRun
{
	public class SuiciderEnemy : Entity
	{
		private Rigidbody2DComponent m_Rigidbody2D;
		private SpriteRendererComponent m_SpriteRenderer;
		private Player m_Player;

		protected override void OnCreate()
		{
			m_Rigidbody2D = GetComponent<Rigidbody2DComponent>();
			m_Player = FindEntityByName("Player").As<Player>();
			m_SpriteRenderer = GetComponent<SpriteRendererComponent>();
		}

		protected override void OnUpdate()
		{
			
		}
	}
}
