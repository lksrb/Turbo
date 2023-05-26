using Turbo;

namespace GunNRun
{
	public class Player : Entity
	{
		public readonly float Speed;
		public readonly float IdleAnimation;
		public readonly float RunningAnimation;

		internal readonly PlayerInput Input = new PlayerInput();
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

			m_ParticleSystem = ParticleSystem.Setup().
				SetColorRange(Vector4.Red).
				SetDurationRange(0.25f, 0.5f).
				Build(this, 20);

			Controller.Init(this);
			Animator.Init(this);
			Gun.Init(this);
		}

		protected override void OnUpdate()
		{
			Input.OnUpdate();
			Controller.OnUpdate();
			Animator.OnUpdate();
			Gun.OnUpdate();

			if (Input.IsShootMouseButtonPressed)
			{
				Gun.Shoot();
			}

			m_ParticleSystem.OnUpdate();
		}
	}
}
