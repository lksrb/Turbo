using Turbo;

namespace Mystery
{
	public class CameraMovement : Entity
	{
		public readonly float Speed;
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
			Transform.Translation = Vector3.Lerp(Transform.Translation, m_Player.Transform.Translation + m_DistanceFromPlayer, Frame.TimeStep * Speed);
		}
	}
}
