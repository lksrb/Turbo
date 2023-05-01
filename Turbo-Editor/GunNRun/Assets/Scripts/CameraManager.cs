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

		protected override void OnCreate()
		{
			Log.Info("Hello from camera!");

			m_PlayerEntity = FindEntityByName("Player");
			m_TopBoundary = FindEntityByName("TopBoundary");
			m_BottomBoundary = FindEntityByName("BottomBoundary");

			if (FollowsPlayer)
			{
				Log.Info($"Player transform: {m_PlayerEntity.Transform.Translation} ");
				Transform.Translation = new Vector3(m_PlayerEntity.Transform.Translation.x, m_PlayerEntity.Transform.Translation.y, Transform.Translation.z);
			}

		}

		protected override void OnUpdate(float ts)
		{
			// Camera moving
			if (FollowsPlayer)
			{
				Vector3 playerTranslation = m_PlayerEntity.Transform.Translation;

				float topY = m_TopBoundary.Transform.Translation.y;
				float bottomY = m_BottomBoundary.Transform.Translation.y;

				// This is the center offset
				topY += -15f;
				bottomY += 15f;
				playerTranslation.y = Mathf.Clamp(playerTranslation.y, bottomY, topY);

				Transform.Translation = Mathf.Lerp(Transform.Translation, playerTranslation, LerpMagnifier * ts);
			}
		}
	}
}
