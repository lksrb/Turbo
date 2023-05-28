using Turbo;

namespace GunNRun
{
	public class Player : Entity
	{
		public readonly float Speed;
		public readonly float IdleAnimation;
		public readonly float RunningAnimation;

		internal readonly PlayerInput PlayerInput = new PlayerInput();
		internal readonly PlayerController Controller = new PlayerController();
		internal readonly PlayerAnimator Animator = new PlayerAnimator();
		internal readonly PlayerGun Gun = new PlayerGun();

		internal Vector2 Velocity => Controller.Velocity;
		internal SpriteRendererComponent SpriteRenderer => GetComponent<SpriteRendererComponent>();

		private CollisionFilter m_Filter = new CollisionFilter();
		private ParticleSystem m_ParticleSystem;

		protected override void OnCreate()
		{
			m_Filter.CollisionCategory = (ushort)EntityCategory.Player;
			m_Filter.CollisionMask = (ushort)EntityCategory.Everything;

			GetComponent<BoxCollider2DComponent>().Filter = m_Filter;

			m_ParticleSystem = ParticleSystem.Setup()
				.SetName("PlayerParticleSystem")
				.AddColor(Color.Red)
				.AddColor(Color.Blue)
				.AddColor(Color.Green)
				.SetDurationRange(0.25f, 0.5f)
				.SetVelocityRange(5, 10)
				.SetRotationVelocityRange(-10.0f, 10f)
				.SetScale(0.15f, 0.15f)
				.Build(20);

			m_ParticleSystem.Start(Transform.Translation);

			Controller.Init(this);
			Animator.Init(this);
			Gun.Init(this);
		}

		protected override void OnUpdate()
		{
			PlayerInput.OnUpdate();
			Controller.OnUpdate();
			Animator.OnUpdate();
			Gun.OnUpdate();

			if (PlayerInput.IsShootMouseButtonPressed)
			{
				Gun.Shoot();
			}
		}
	}
}
