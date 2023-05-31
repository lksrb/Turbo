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

		private WaveManager m_WaveManager;
		
		// Text
		private Entity m_LevelText;
		private TextComponent m_LevelTextComponent;
		private Vector4 m_DefaultTextColor;
		private bool m_TextFollowsCamera = true;
		private TransformComponent m_CameraTransform;

		private AudioSourceComponent m_BackgroundMusic;
		
		protected override void OnCreate()
		{
			// Input.SetCursorMode(CursorMode.Hidden);

			m_WaveManager = new WaveManager(this);
			m_WaveManager.OnChangeWaveState += OnChangeWaveState;

			m_LevelText = FindEntityByName("LevelText");
			m_CameraTransform = FindEntityByName("Camera").Transform;

			m_LevelTextComponent = m_LevelText.GetComponent<TextComponent>();
			m_DefaultTextColor = m_LevelTextComponent.Color;

			m_BackgroundMusic = GetComponent<AudioSourceComponent>();
		}

		private void OnChangeWaveState(WaveState state)
		{
			switch (state)
			{
				case WaveState.WaitingForNextWave:
					m_TextFollowsCamera = true;
					m_LevelTextComponent.Color = m_DefaultTextColor;
					m_LevelTextComponent.Text = "Wave " + m_WaveManager.CurrentWave.ToString();
					break;
				case WaveState.Wave:
					m_BackgroundMusic.Play();
					m_TextFollowsCamera = false;
					m_LevelTextComponent.Color = new Vector4(m_LevelTextComponent.Color.XYZ, 0.0f);
					break;
				case WaveState.WaveFinished:
					break;
			}
		}

		protected override void OnUpdate()
		{
			if (m_TextFollowsCamera)
			{
				Vector3 translation = new Vector3(m_CameraTransform.Translation.XY, 2.0f);
				translation.X -= 1.5f;
				m_LevelText.Transform.Translation = translation;
			}

			m_WaveManager.OnUpdate();
		}
	}
}
