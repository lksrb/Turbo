using Turbo;

namespace Mystery
{
	internal class ConveyorBelt : Entity
	{
		Entity m_Remember = null;
		Vector3 m_LastTranslation = Vector3.Zero;
		Vector3 m_LinearVelocity;
		
		protected override void OnCreate()
		{
			OnCollisionBegin += OnCollisionStart;
			OnCollisionEnd += OnCollisionStop;

			m_LastTranslation = Transform.Translation;
		}


		protected override void OnUpdate()
		{
			if(m_Remember != null)
			{
				m_Remember.GetComponent<RigidbodyComponent>().Position += Vector3.Right * Frame.TimeStep * 5.0f;

				m_LinearVelocity = (m_Remember.GetComponent<RigidbodyComponent>().Position - m_LastTranslation) / Frame.TimeStep;
				m_LastTranslation = m_Remember.GetComponent<RigidbodyComponent>().Position;
			}
		}

		void OnCollisionStart(Entity entity)
		{
			if (entity.Name == "Player")
				return;

			m_Remember = entity;
		}

		void OnCollisionStop(Entity entity)
		{
			if(entity == m_Remember)
			{
				m_Remember.GetComponent<RigidbodyComponent>().LinearVelocity = m_LinearVelocity;
				m_Remember = null;
			}

		}
	}
}
