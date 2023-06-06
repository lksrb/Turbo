using Turbo;

namespace GunNRun
{
	internal class DropItem : Entity
	{
		private Player m_Player;

		protected override void OnCreate()
		{
			m_Player = FindEntityByName("Player").As<Player>();
		}

		protected override void OnUpdate()
		{
			if(Mathf.Length(m_Player.Transform.Translation - Transform.Translation) < 1.0f)
			{
				m_Player.PickItem(this);
				Scene.DestroyEntity(this);
			}
		}
	}
}
