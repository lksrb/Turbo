using Turbo;

namespace Mystery
{
	public class BouncyBall : Entity
	{
		private Entity m_Owner;
		private bool m_CanDamage = false;
		private RigidbodyComponent m_Rigidbody;
		private Prefab m_BouncyBallPrefab;
		private Player m_Player;
		private bool m_CanSpawnAnotherBall = false;	

		protected override void OnCreate()
		{
			m_Rigidbody = GetComponent<RigidbodyComponent>();

			OnCollisionBegin += BouncyBall_OnCollisionBegin;
			OnCollisionEnd += BouncyBall_OnCollisionEnd;
			m_BouncyBallPrefab = Assets.LoadPrefab("Prefabs/BouncyBall.tprefab");

			m_Player = FindEntityByName("Player").As<Player>();
		}

		protected override void OnUpdate()
		{
			if (m_Owner == null)
			{
				m_CanDamage = m_Rigidbody.LinearVelocity.Length() > 5.0f;
			}
		}

		internal bool SetOwner(Entity entity)
		{
			if (m_Owner == null)
			{
				m_CanSpawnAnotherBall = true;
				m_CanDamage = false;
				m_Owner = entity;
				return true;
			}

			return false;
		}

		internal void Release(Entity entity)
		{
			if (m_Owner == entity)
			{
				m_CanDamage = true;
				m_Owner = null;
			}
		}

		internal Entity Owner => m_Owner;

		internal bool CanDamage => m_CanDamage;

		private void BouncyBall_OnCollisionBegin(Entity entity)
		{
			
		}

		private void BouncyBall_OnCollisionEnd(Entity entity)
		{
			if (m_CanSpawnAnotherBall && m_CanDamage && entity.Name == "Wall")
			{
				m_CanSpawnAnotherBall = false;

				var linearVelocity = m_Rigidbody.LinearVelocity;
				BouncyBall bouncyBall = Instantiate(m_BouncyBallPrefab, Transform.Translation).As<BouncyBall>();
				bouncyBall.m_Rigidbody.LinearVelocity = Vector3.Normalize(m_Player.CurrentPosition - bouncyBall.Transform.Translation) * linearVelocity.Length();
				m_Player.BouncyBalls.Add(bouncyBall);
			}
		}
	}
}
