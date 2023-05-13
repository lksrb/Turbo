using Turbo;

namespace GunNRun
{
	public class CameraManager : Entity
	{
		public readonly bool FollowsPlayer;
		public readonly float LerpMagnifier;
		public readonly float ShakeDuration = 0.3f; // Duration of the shake effect
		public readonly float ShakeMagnitude = 0.1f; // Intensity of the shake effect

		private Player m_Player;
		private Entity m_TopBoundary;
		private Entity m_BottomBoundary;
		private TransformComponent m_PlayerTransform;

		// Camera shake
		private Vector3 m_BeforeShakeTranslation = Vector3.Zero;

		private float m_CurrentShakeDuration = 0.0f; // Current duration of the shake effect
		private float m_CurrentShakeMagnitude = 0.0f; // Current intensity of the shake effect

		protected override void OnCreate()
		{
			Log.Info("Hello from camera!");

			m_Player = FindEntityByName("Player").As<Player>();
			//m_TopBoundary = FindEntityByName("TopBoundary");
			//m_BottomBoundary = FindEntityByName("BottomBoundary");

			m_PlayerTransform = m_Player.Transform;

			if (FollowsPlayer)
			{
				Transform.Translation = new Vector3(m_PlayerTransform.Translation.XY, Transform.Translation.Z);
			}
		}

		protected override void OnUpdate()
		{
			// Camera moving
			if (FollowsPlayer)
			{
				if (m_CurrentShakeDuration > 0.0f)
				{
					SimpleCameraShake();
				}
				else
				{
					Vector3 playerTranslation = new Vector3(m_PlayerTransform.Translation.XY, Transform.Translation.Z);

					/*	float topY = m_TopBoundary.Transform.Translation.Y;
						float bottomY = m_BottomBoundary.Transform.Translation.Y;

						// This is the center offset
						topY += -13f;
						bottomY += 13f;
						playerTranslation.Y = Mathf.Clamp(playerTranslation.Y, bottomY, topY);*/
					m_BeforeShakeTranslation = Transform.Translation = Mathf.Lerp(Transform.Translation, playerTranslation, LerpMagnifier * Frame.TimeStep);
				}
			}

			if(m_Player.Gun.BulletShot)
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

		private void AdvancedCameraShake()
		{
			Vector2 offset = Random.InsideUnitCircle() * m_CurrentShakeMagnitude;
			Transform.Translation = m_BeforeShakeTranslation + offset;
			m_CurrentShakeDuration -= Frame.TimeStep;
		}
	}
}
