using Turbo;
using static Mystery.PlayerLayer;

namespace Mystery
{
	public enum PlayerEvent : uint
	{
		OnBallPicked = 0,
		OnBallThrew,
	}

	public class Player : Entity
	{
		public float LinearVelocityMagnifier = 0.0f;
		public float AngularVelocityMagnifier = 0.0f;
		public float PickLength = 0.0f;
		public float PickHeight = 0.0f;
		public float PickUpRadius = 0.0f;

		private PlayerLayerSystem m_LayerSystem;

		private Entity m_TargetCrosshair;

		Entity m_Hat;

		protected override void OnCreate()
		{
			m_LayerSystem = new PlayerLayerSystem(this, 3);

			m_LayerSystem.PushLayer<PlayerInput>();
			m_LayerSystem.PushLayer<PlayerMovement>().RayCastHit += OnRayCastHit;
			m_LayerSystem.PushLayer<PlayerBallPick>();

			// Establish event connections
			m_LayerSystem.Listen<PlayerMovement>().To<PlayerBallPick>();

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

		void OnRayCastHit(Vector3 hitPosition)
		{
			var transform = m_TargetCrosshair.Transform;
			transform.Translation = hitPosition + Vector3.Up * 0.03f;
			transform.Rotation = Vector3.Right * Mathf.Radians(90.0f);
		}
	}
}
