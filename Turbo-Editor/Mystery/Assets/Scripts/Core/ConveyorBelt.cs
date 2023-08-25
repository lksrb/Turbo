using System.Collections.Generic;
using Turbo;

namespace Mystery
{
	internal class ConveyorBelt : Entity
	{
		class Velocity
		{
			public Vector3 LinearVelocity = Vector3.Zero;
			public Vector3 LastTranslation = Vector3.Zero;
		}

		Dictionary<Entity, Velocity> m_LinearVelocities = new Dictionary<Entity, Velocity>();
		List<Entity> m_OnBelt = new List<Entity>();

		protected override void OnCreate()
		{
			OnCollisionBegin += OnCollisionStart;
			OnCollisionEnd += OnCollisionStop;
		}

		protected override void OnUpdate()
		{
			foreach (var entity in m_OnBelt)
			{
				entity.GetComponent<RigidbodyComponent>().Position += Vector3.Right * Frame.TimeStep * 5.0f;

				var velocity = m_LinearVelocities[entity];
				velocity.LinearVelocity = (entity.GetComponent<RigidbodyComponent>().Position - velocity.LastTranslation) / Frame.TimeStep;
				velocity.LastTranslation = entity.GetComponent<RigidbodyComponent>().Position;
			}
			
		}

		void OnCollisionStart(Entity entity)
		{
			if (m_OnBelt.Contains(entity))
				return;

			m_OnBelt.Add(entity);

			m_LinearVelocities[entity].LastTranslation = entity.Transform.Translation;
		}

		void OnCollisionStop(Entity entity)
		{
			if (m_OnBelt.Contains(entity))
			{
				entity.GetComponent<RigidbodyComponent>().LinearVelocity = m_LinearVelocities[entity].LinearVelocity;
				m_OnBelt.Remove(entity);
			}
		}
	}
}
