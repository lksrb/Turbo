using Turbo;

namespace Mystery
{
	public class CameraMovement : Entity
	{
		public float Speed;

		private Vector3 m_MovementDirection = Vector3.Zero;
		private Entity m_Player;
		private Vector3 m_DistanceFromPlayer;

		protected override void OnCreate()
		{
			m_Player = FindEntityByName("Player");

			// To keep the parent-child relationship editor-only
			UnParent();

			m_DistanceFromPlayer = Transform.Translation - m_Player.Transform.Translation;

		}

		protected override void OnUpdate()
		{
			OnMovement();
		}

		private void OnMovement()
		{
			m_MovementDirection = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W))
			{
				m_MovementDirection.Z = -1;
			}

			if (Input.IsKeyDown(KeyCode.S))
			{
				m_MovementDirection.Z = 1;
			}
			if (Input.IsKeyDown(KeyCode.A))
			{
				m_MovementDirection.X = -1;
			}

			if (Input.IsKeyDown(KeyCode.D))
			{
				m_MovementDirection.X = 1;
			}

			Vector3 transform = m_Player.Transform.Translation + m_DistanceFromPlayer;
			transform.Y = Transform.Translation.Y;

			Transform.Translation = Vector3.Lerp(Transform.Translation, transform, Frame.TimeStep * 5.0f);
		}
	}
}
