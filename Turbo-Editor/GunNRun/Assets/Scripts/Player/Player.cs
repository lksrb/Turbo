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
		protected override void OnCreate()
		{
			m_Filter.CollisionCategory = (ushort)GameCategory.Player;
			m_Filter.CollisionMask = (ushort)GameCategory.Everything;

			GetComponent<BoxCollider2DComponent>().Filter = m_Filter;

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

			if(Input.IsShootMouseButtonPressed)
			{
				Gun.Shoot();
			}
		}
	}
}
