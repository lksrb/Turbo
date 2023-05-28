using System.Collections.Generic;
using Turbo;

namespace GunNRun
{
	internal enum EntityCategory : uint
	{
		Bullet = 1 << 1,
		Player = 1 << 2,
		Enemy = 1 << 3,
		Wall = 1 << 4,
		Everything = 0xFFFF
	}

	internal interface IGameManagerModule
	{
		void Init(GameManager manager);
		void OnUpdate();
		void OnNewWave();
	}

	internal class GameManager : Entity
	{
		private class ModuleManager
		{
			private List<IGameManagerModule> m_Modules = new List<IGameManagerModule>();

			internal T AddModule<T>() where T : IGameManagerModule, new()
			{
				T module = new T();
				m_Modules.Add(module);
				return module;
			}

			internal void InitModules(GameManager manager)
			{
				foreach (var module in m_Modules)
				{
					module.Init(manager);
				}
			}

			internal void OnUpdate()
			{
				foreach (var module in m_Modules)
				{
					module.OnUpdate();
				}
			}
			internal void OnNewWave()
			{
				foreach (var module in m_Modules)
				{
					module.OnNewWave();
				}
			}

			internal T Get<T>() where T : IGameManagerModule
			{
				foreach (var module in m_Modules)
				{
					if(module is T)
					{
						return (T)module;
					}
				}

				Log.Error("No module found!");

				return default(T);
			}
		}

		private ModuleManager m_ModuleManager = new ModuleManager();
		private LevelManager m_LevelManager;
		
		// Text
		private Entity m_LevelText;
		private TextComponent m_LevelTextComponent;
		private Vector4 m_DefaultTextColor;
		private bool m_TextFollowsCamera = true;
		private Entity m_Camera;

		// Wave timer
		private SingleTickTimer m_NewWaveTimer = new SingleTickTimer(4.0f);

		private AudioSourceComponent m_BackgroundMusic;
		
		protected override void OnCreate()
		{
			// Input.SetCursorMode(CursorMode.Hidden);

			m_LevelManager = m_ModuleManager.AddModule<LevelManager>();
			m_ModuleManager.AddModule<AudioManager>();
			m_LevelManager.OnChangeLevelState += OnChangeLevelState;

			m_ModuleManager.InitModules(this);

			m_LevelText = FindEntityByName("LevelText");
			m_Camera = FindEntityByName("Camera");

			m_LevelTextComponent = m_LevelText.GetComponent<TextComponent>();
			m_DefaultTextColor = m_LevelTextComponent.Color;

			m_BackgroundMusic = GetComponent<AudioSourceComponent>();
		}

		private void OnChangeLevelState(LevelState state)
		{
			switch (state)
			{
				case LevelState.WaitingForNextWave:
					m_NewWaveTimer.Reset();
					m_TextFollowsCamera = true;
					m_LevelTextComponent.Color = m_DefaultTextColor;
					m_LevelTextComponent.Text = "Level " + m_LevelManager.CurrentLevel.ToString();
					break;
				case LevelState.Wave:
					m_BackgroundMusic.Play();
					m_TextFollowsCamera = false;
					break;
				case LevelState.WaveFinished:
					break;
			}
		}

		protected override void OnUpdate()
		{
			if (m_NewWaveTimer)
			{
				m_LevelTextComponent.Color = new Vector4(m_LevelTextComponent.Color.XYZ, 0.0f);
				m_ModuleManager.OnNewWave();
			}

			if (m_TextFollowsCamera)
			{
				Vector3 translation = new Vector3(m_Camera.Transform.Translation.XY, 2.0f);
				translation.X -= 1.5f;
				m_LevelText.Transform.Translation = translation;
			}

			m_ModuleManager.OnUpdate();
		}

		internal LevelManager GetLevelManager() => m_LevelManager;
	}
}
