using System;
using Turbo;

namespace GunNRun
{
	public class CameraManager : Entity
	{
		public bool FollowsPlayer;
		public float LerpMagnifier;

		private Player m_Player;
		private Entity m_TopBoundary;
		private Entity m_BottomBoundary;
		private TransformComponent m_PlayerTransform;

		// Camera shake
		private Vector3 m_BeforeShakeTranslation = Vector3.Zero;

		private float shakeDuration = 0.5f; // Duration of the shake effect
		private float shakeMagnitude = 0.1f; // Intensity of the shake effect
		private float currentShakeDuration = 0.0f; // Current duration of the shake effect
		private float currentShakeMagnitude = 0.0f; // Current intensity of the shake effect
		private Random m_Random = new Random();

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

			/// First one is slow
			m_Random.Next();
		}

		protected override void OnUpdate(float ts)
		{
			// Camera moving
			if (FollowsPlayer)
			{
				if (currentShakeDuration > 0)
				{
					SimpleCameraShake(ts);
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
					m_BeforeShakeTranslation = Transform.Translation = Mathf.Lerp(Transform.Translation, playerTranslation, LerpMagnifier * ts);
				}
			}

			// Camera shake
			if (m_Player.Input.IsShootMouseButtonPressed)
			{
				currentShakeDuration = 0.3f;
				currentShakeMagnitude = 0.2f;
			}
		}

		private float RandomFloat(float minValue, float maxValue)
		{
			return minValue + (float)m_Random.NextDouble() * (maxValue - minValue);
		}

		private void SimpleCameraShake(float ts)
		{
			float offsetX = RandomFloat(-currentShakeMagnitude, currentShakeMagnitude);
			float offsetY = RandomFloat(-currentShakeMagnitude, currentShakeMagnitude);

			Vector3 translation = m_BeforeShakeTranslation;
			translation.X = m_BeforeShakeTranslation.X + offsetX;
			translation.Y = m_BeforeShakeTranslation.Y + offsetY;
			Transform.Translation = translation;

			currentShakeDuration -= ts;
			currentShakeMagnitude = Mathf.Lerp(0.0f, shakeMagnitude, currentShakeDuration / shakeDuration);
		}

		private void AdvancedCameraShake(float ts)
		{
			Vector3 offset = Vector3.Zero;
			offset.X = RandomFloat(-1, 1);
			offset.Y = RandomFloat(-1, 1);

			offset *= currentShakeMagnitude;

			Transform.Translation = m_BeforeShakeTranslation + offset;
			currentShakeDuration -= ts;
		}
	}
}
