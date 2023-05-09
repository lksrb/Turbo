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
		internal readonly GunManager Gun = new GunManager();

		internal Vector2 Velocity => Controller.Velocity;
		internal SpriteRendererComponent SpriteRenderer => GetComponent<SpriteRendererComponent>();

		protected override void OnCreate()
		{
			Controller.Init(this);
			Animator.Init(this);
			Gun.Init(this);
		}

		protected override void OnUpdate(float ts)
		{
			Input.OnUpdate();
			Controller.OnUpdate(ts);
			Animator.OnUpdate(ts);
			Gun.OnUpdate(ts);

			if(Input.IsShootMouseButtonPressed)
			{
				Gun.Shoot();
			}
		}
	}
}
