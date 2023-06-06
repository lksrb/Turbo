using Turbo;

namespace GunNRun
{
	public class CameraManager : Entity
	{
		public bool FollowsPlayer;
		public readonly float LerpMagnifier;
		public readonly float ShakeDuration = 0.3f; // Duration of the shake effect
		public readonly float ShakeMagnitude = 0.1f; // Intensity of the shake effect

		private Player m_Player;
		private TransformComponent m_PlayerTransform;

		private bool m_CameraShake = true;

		// Camera shake
		private Vector3 m_BeforeShakeTranslation = Vector3.Zero;

		private float m_CurrentShakeDuration = 0.0f; // Current duration of the shake effect
		private float m_CurrentShakeMagnitude = 0.0f; // Current intensity of the shake effect

		private GameManager m_GameManager;

		// Camera extend
		private Entity m_Crosshair;

		protected override void OnCreate()
		{
			Log.Info("Hello from camera!");

			m_Player = FindEntityByName("Player").As<Player>();
			m_Crosshair = FindEntityByName("Crosshair");

			m_PlayerTransform = m_Player.Transform;

			if (FollowsPlayer)
			{
				Transform.Translation = new Vector3(m_PlayerTransform.Translation.XY, Transform.Translation.Z);
			}

			m_GameManager = FindEntityByName("GameManager").As<GameManager>();
		}

		protected override void OnUpdate()
		{
			if (m_GameManager.CurrentGameState == GameState.GameOver)
			{
				FollowsPlayer = false;
			}

			// Camera moving
			if (FollowsPlayer)
			{
				if (m_CurrentShakeDuration > 0.0f)
				{
					SimpleCameraShake();
				}
				else
				{
					Vector3 player = new Vector3(m_PlayerTransform.Translation.XY);
					Vector3 crosshair = m_Crosshair.Transform.Translation;

					Vector3 distance = player + (crosshair - player) * 0.3f;

					distance.Z = Transform.Translation.Z;
					m_BeforeShakeTranslation = Transform.Translation = Mathf.Lerp(Transform.Translation, distance, LerpMagnifier * Frame.TimeStep);
				}
			}

			if (m_CameraShake && m_Player.Gun.BulletShot)
			{
				m_CurrentShakeDuration = ShakeDuration;
				m_CurrentShakeMagnitude = ShakeMagnitude;
			}
		}

		private void SimpleCameraShake()
		{
			Vector2 offset = Random.Float2(-m_CurrentShakeMagnitude, m_CurrentShakeMagnitude);
			Transform.Translation = m_BeforeShakeTranslation + offset;

			m_CurrentShakeDuration -= Frame.TimeStep;
			m_CurrentShakeMagnitude = Mathf.Lerp(0.0f, ShakeMagnitude, m_CurrentShakeDuration / ShakeDuration);
		}
	}
}
