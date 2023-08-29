using System.Collections.Generic;
using Turbo;

namespace Mystery
{
	internal class ConveyorBelt : Entity
	{
		public float BeltSpeed = 0.0f;

		List<Entity> m_OnBelt = new List<Entity>();

		Vector3 m_Right;

		protected override void OnCreate()
		{
			OnCollisionBegin += OnCollisionStart;
			OnCollisionEnd += OnCollisionStop;

			m_Right = new Quaternion(Transform.Rotation) * Vector3.Right;
			m_Right.Normalize();
		}

		protected override void OnUpdate()
		{
			foreach (var entity in m_OnBelt)
			{
				entity.GetComponent<RigidbodyComponent>().Position += m_Right * Frame.TimeStep * BeltSpeed;
			}
		}

		void OnCollisionStart(Entity entity)
		{
			if (entity.Name == "Player")
				return;

			if (m_OnBelt.Contains(entity))
				return;

			m_OnBelt.Add(entity);
		}

		void OnCollisionStop(Entity entity)
		{
			if (m_OnBelt.Contains(entity))
			{
				m_OnBelt.Remove(entity);
				entity.GetComponent<RigidbodyComponent>().LinearVelocity += m_Right * 3.0f;
			}
		}
	}
}
