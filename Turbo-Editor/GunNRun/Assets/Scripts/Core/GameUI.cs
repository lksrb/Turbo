using Turbo;

namespace GunNRun
{
	public class GameUI
	{
		private Player m_Player;
		private bool m_WaveTextFollowCamera = true;
		private GameManager m_GameManager;
		private TransformComponent m_CameraTransform;

		private Vector4 m_DefaultTextWaveColor;

		private TextComponent m_PlayerHPText, m_PlayerAmmoText, m_WaveTextComponent, m_ScoreTextComponent;
		private Entity m_PlayerHP, m_PlayerAmmo, m_WaveText, m_ScoreText;
		private readonly Vector3 m_PlayerHPOffset, m_PlayerAmmoOffset, m_WaveTextOffset, m_ScoreTextOffset;

		internal GameUI(GameManager manager)
		{
			m_GameManager = manager;
			manager.WaveManager.OnChangeWaveState += OnChangeState;

			m_Player = manager.FindEntityByName("Player").As<Player>();
			m_CameraTransform = manager.FindEntityByName("Camera").Transform;

			m_WaveText = manager.FindEntityByName("Wave-Text");
			m_WaveTextComponent = m_WaveText.GetComponent<TextComponent>();
			m_DefaultTextWaveColor = m_WaveTextComponent.Color;

			m_ScoreText = manager.FindEntityByName("Score-Text");
			m_ScoreTextComponent = m_ScoreText.GetComponent<TextComponent>();

			m_PlayerHP = m_Player.FindEntityByName("HP-Text");
			m_PlayerAmmo = m_Player.FindEntityByName("Ammo-Text");

			m_PlayerHPText = m_PlayerHP.GetComponent<TextComponent>();
			m_PlayerAmmoText = m_PlayerAmmo.GetComponent<TextComponent>();

			m_PlayerHPOffset = new Vector3(6.0f, -4.5f, 1.0f);
			m_PlayerAmmoOffset = new Vector3(-8.0f, -4.5f, 1.0f);
			m_WaveTextOffset = new Vector3(-1.0f, 0.0f, 2.0f);
			m_ScoreTextOffset = new Vector3(6.0f, 4.5f, 1.0f);

			m_GameManager = manager.FindEntityByName("GameManager").As<GameManager>();
		}

		internal void OnUpdate()
		{
			if (m_Player.HP <= 0)
			{
				m_WaveTextComponent.Color = m_DefaultTextWaveColor;
				m_WaveTextComponent.Text = "Game Over!";
			}

			// Wave text
			if (m_WaveTextFollowCamera)
			{
				m_WaveText.Transform.Translation = m_CameraTransform.Translation + m_WaveTextOffset;
			}

			// Ammo & HP text
			m_PlayerHP.Transform.Translation = m_CameraTransform.Translation + m_PlayerHPOffset;
			m_PlayerAmmo.Transform.Translation = m_CameraTransform.Translation + m_PlayerAmmoOffset;

			m_PlayerHPText.Text = m_Player.HP.ToString();
			m_PlayerAmmoText.Text = m_Player.AmmoCount.ToString();

			// Score text
			m_ScoreText.Transform.Translation = m_CameraTransform.Translation + m_ScoreTextOffset;
			m_ScoreTextComponent.Text = $"Score\n{ m_Player.ScoreCount }";
		}

		private void OnChangeState(WaveState state)
		{
			switch (state)
			{
				case WaveState.WaitingForNextWave:
					m_WaveTextFollowCamera = true;
					m_WaveTextComponent.Color = m_DefaultTextWaveColor;
					m_WaveTextComponent.Text = "Wave " + m_GameManager.WaveManager.CurrentWave.ToString();
					break;
				case WaveState.Wave:
					m_WaveTextFollowCamera = false;
					m_WaveTextComponent.Color = new Vector4(m_WaveTextComponent.Color.XYZ, 0.0f);
					break;
				case WaveState.WaveFinished:
					break;
			}
		}
	}

}
