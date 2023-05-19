using Turbo;

namespace GunNRun
{
	public enum GameCategory : uint
	{
		Bullet = 1 << 1,
		Player = 1 << 2,
		Enemy = 1 << 3,
		Wall = 1 << 4,
		Everything = 0xFFFF
	}

	internal class GameManager : Entity
	{
		internal EnemyManager Enemies = new EnemyManager();
		private LevelManager m_LevelManager = new LevelManager();
		private Entity m_LevelText;
		private TextComponent m_LevelTextComponent;
		private Entity m_Camera;
		private Timer m_ShowLevelText = new Timer(4.0f, false);

		protected override void OnCreate()
		{
			// Input.SetCursorMode(CursorMode.Hidden);

			Enemies.Init(this);
			m_LevelManager.Init(this);

			m_LevelText = FindEntityByName("LevelText");
			m_Camera = FindEntityByName("Camera");

			m_LevelTextComponent = m_LevelText.GetComponent<TextComponent>();
		}

		protected override void OnUpdate()
		{
			if (!m_ShowLevelText)
			{
				Vector3 translation = new Vector3(m_Camera.Transform.Translation.XY, 2.0f);
				translation.X -= 1.5f;
				m_LevelText.Transform.Translation = translation;
			}
			else
			{
				m_LevelTextComponent.Color = new Vector4(m_LevelTextComponent.Color.XYZ, 0.0f);
			}

			m_LevelManager.OnUpdate();
		}
	}
}
