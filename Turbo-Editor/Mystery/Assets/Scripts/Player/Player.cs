using Turbo;
using static Mystery.PlayerModule;

namespace Mystery
{
	public enum PlayerEvent : uint
	{
		OnBallPicked = 0,
		OnBallThrew,
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

		private PlayerModuleSystem m_Modules;

		private Entity m_TargetCrosshair;

		Entity m_Hat;

		protected override void OnCreate()
		{
			m_Modules = new PlayerModuleSystem(this, 3);

			m_Modules.AttachModule<PlayerInput>();
			m_Modules.AttachModule<PlayerMovement>().m_OnChangeTargetLocation += OnChangeTargetLocation;
			m_Modules.AttachModule<PlayerBallPick>();

			// Establish event connections
			m_Modules.Listen<PlayerMovement>().To<PlayerBallPick>();

			m_Hat = FindEntityByName("Hat");
			m_TargetCrosshair = FindEntityByName("TargetCrosshair");

			Log.Info("Hello entity!");
		}

		protected override void OnUpdate()
		{
			m_Modules.OnUpdate();

			// Hat
			var r = m_Hat.Transform.Rotation;
			r.XY -= Frame.TimeStep;
			m_Hat.Transform.Rotation = r;
		}

		void OnChangeTargetLocation(Vector3 hitPosition)
		{
			var transform = m_TargetCrosshair.Transform;
			transform.Translation = hitPosition + Vector3.Up * 0.03f;
			transform.Rotation = Vector3.Right * Mathf.Radians(90.0f);
		}
	}
}
