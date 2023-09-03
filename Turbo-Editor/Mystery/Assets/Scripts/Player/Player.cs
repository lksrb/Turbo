using Turbo;
using static Mystery.Layer<Mystery.Player, Mystery.PlayerEvent>;

namespace Mystery
{
	public enum PlayerEvent : uint
	{
		BallGrabbed = 0,
		BallThrew,
	}

	public class Player : Entity
	{
		public float LinearVelocityMagnifier;
		public readonly float AngularVelocityMagnifier;
		public readonly float PickLength;
		public readonly float PickHeight;
		public readonly float PickUpRadius;
		public readonly float LockMovementDeltaMagnifier;
		public readonly float BallThrowPower;

		private LayerSystem<Player, PlayerEvent> m_LayerSystem;

		private Entity m_TargetCrosshair, m_Hat;

		protected override void OnCreate()
		{
			m_LayerSystem = new LayerSystem<Player, PlayerEvent>(this, 3);

			m_LayerSystem.PushLayer<PlayerLayer>();
			m_LayerSystem.PushLayer<PlayerInput>();
			m_LayerSystem.PushLayer<PlayerMovement>().m_OnChangeTargetLocation += OnChangeTargetLocation;
			m_LayerSystem.PushLayer<PlayerBallThrow>();

			// Establish event connections
			m_LayerSystem.Listen<PlayerMovement>().To<PlayerBallThrow>();
			m_LayerSystem.Listen<PlayerLayer>().To<PlayerBallThrow>();

			m_Hat = FindEntityByName("Hat");
			m_TargetCrosshair = FindEntityByName("TargetCrosshair");

			Log.Info("Hello entity!");
		}

		protected override void OnUpdate()
		{
			m_LayerSystem.OnUpdate();

			// Hat
			var r = m_Hat.Transform.Rotation;
			r.XY -= Frame.TimeStep;
			m_Hat.Transform.Rotation = r;
		}

		private void OnChangeTargetLocation(Vector3 hitPosition)
		{
			var transform = m_TargetCrosshair.Transform;
			transform.Translation = hitPosition + Vector3.Up * 0.03f;
			transform.Rotation = Vector3.Right * Mathf.Radians(90.0f);
		}
	}

	internal class PlayerLayer : Layer<Player, PlayerEvent>
	{
		protected override void OnEvent(PlayerEvent layerEvent)
		{
			EnemyManager.Emit(layerEvent);
		}
	}
}
