using Turbo;

namespace GunNRun
{
	public class CameraManager : Entity
	{
		// Set in editor
		public readonly bool FollowsPlayer;
		public readonly float LerpMagnifier;
		public readonly float ShakeDuration = 0.3f; // Duration of the shake effect
		public readonly float ShakeMagnitude = 0.1f; // Intensity of the shake effect

		// Camera shake
		private const bool m_CameraShake = true;
		private float m_CurrentShakeDuration = 0.0f; // Current duration of the shake effect
		private float m_CurrentShakeMagnitude = 0.0f; // Current intensity of the shake effect
		private Vector3 m_BeforeShakeTranslation = Vector3.Zero;

		// Boundaries
		private Vector2 m_TopRight, m_BottomLeft;

		// Signal bullet shot
		private Player m_Player;

		// Camera extend
		private Entity m_Crosshair;

		protected override void OnCreate()
		{
			m_Player = FindEntityByName("Player").As<Player>();
			m_Crosshair = FindEntityByName("Crosshair");

			m_TopRight = FindEntityByName("CameraTopRightBoundary").Transform.Translation;
			m_BottomLeft = FindEntityByName("CameraBottomLeft").Transform.Translation;

			if (FollowsPlayer)
			{
				Transform.Translation = new Vector3(m_Player.Transform.Translation.XY, Transform.Translation.Z);
			}

			Log.Warn("Camera initialized!");
		}

		protected override void OnUpdate()
		{
			// Camera moving
			if (FollowsPlayer)
			{
				if (m_CurrentShakeDuration > 0.0f)
				{
					Vector2 offset = Random.Float2(-m_CurrentShakeMagnitude, m_CurrentShakeMagnitude);
					Transform.Translation = m_BeforeShakeTranslation + offset;

					m_CurrentShakeDuration -= Frame.TimeStep;
					m_CurrentShakeMagnitude = Mathf.Lerp(0.0f, ShakeMagnitude, m_CurrentShakeDuration / ShakeDuration);
				}
				else
				{
					Vector2 player = m_Player.Transform.Translation.XY;
					Vector2 crosshair = m_Crosshair.Transform.Translation;
					Vector2 cameraExtend = player + (crosshair - player) * 0.3f;

					Vector3 finalTranslation = Vector3.Zero;
					finalTranslation.XY = Mathf.Clamp(cameraExtend, m_BottomLeft, m_TopRight);
					finalTranslation.Z = Transform.Translation.Z;
					m_BeforeShakeTranslation = Transform.Translation = Mathf.Lerp(Transform.Translation, finalTranslation, LerpMagnifier * Frame.TimeStep);
				}
			}

			if (m_CameraShake && m_Player.Gun.BulletShot)
			{
				m_CurrentShakeDuration = ShakeDuration;
				m_CurrentShakeMagnitude = ShakeMagnitude;
			}
		}
	}
}
