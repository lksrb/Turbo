using Turbo;

namespace GunNRun
{
	public class CameraManager : Entity
	{
		public bool FollowsPlayer;
		public float LerpMagnifier;

		private Entity m_PlayerEntity;
		private Entity m_TopBoundary;
		private Entity m_BottomBoundary;
		private TransformComponent m_PlayerTransform;

		protected override void OnCreate()
		{
			Log.Info("Hello from camera!");

			m_PlayerEntity = FindEntityByName("Player");
			m_TopBoundary = FindEntityByName("TopBoundary");
			m_BottomBoundary = FindEntityByName("BottomBoundary");

			m_PlayerTransform = m_PlayerEntity.Transform;

			if (FollowsPlayer)
			{
				Log.Info($"Player transform: {m_PlayerTransform} ");
				Transform.Translation = new Vector3(m_PlayerTransform.Translation.XY, Transform.Translation.Z);
			}

		}

		protected override void OnUpdate(float ts)
		{
			// Camera moving
			if (FollowsPlayer)
			{
				Vector3 playerTranslation = m_PlayerTransform.Translation;

				float topY = m_TopBoundary.Transform.Translation.Y;
				float bottomY = m_BottomBoundary.Transform.Translation.Y;

				// This is the center offset
				topY += -13f;
				bottomY += 13f;
				playerTranslation.Y = Mathf.Clamp(playerTranslation.Y, bottomY, topY);

				Transform.Translation = Mathf.Lerp(Transform.Translation, playerTranslation, LerpMagnifier * ts);
			}
		}
	}
}
