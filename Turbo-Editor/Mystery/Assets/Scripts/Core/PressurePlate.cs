using System.Collections.Generic;
using Turbo;

namespace Mystery
{
	internal class PressurePlate : Entity
	{
		bool m_StepOn = false;

		RigidbodyComponent m_Rigidbody;

		Vector3 m_DefaultPosition;

		Prefab[] m_DeliveryVariants = new Prefab[3];

		protected override void OnCreate()
		{
			OnTriggerBegin += OnPlayerStepOn;
			OnTriggerEnd += OnPlayerStepOff;

			m_Rigidbody = GetComponent<RigidbodyComponent>();
			m_DefaultPosition = m_Rigidbody.Position;

			m_DeliveryVariants[0] = Assets.LoadPrefab("Prefabs/DeliveryBox.tprefab");
			m_DeliveryVariants[1] = Assets.LoadPrefab("Prefabs/BouncyBall.tprefab");
			m_DeliveryVariants[2] = Assets.LoadPrefab("Prefabs/DeliveryCapsule.tprefab");
		}

		protected override void OnUpdate()
		{
			// Minimizing DLL calls
			var currentPosition = m_Rigidbody.Position;

			if (m_StepOn)
			{
				currentPosition.Y = Mathf.Cerp(currentPosition.Y, m_DefaultPosition.Y - Transform.Scale.Y * 0.30f, Frame.TimeStep * 3.0f);
			} else
			{
				currentPosition.Y = Mathf.Cerp(currentPosition.Y, m_DefaultPosition.Y, Frame.TimeStep * 3.0f);
			}

			m_Rigidbody.Position = currentPosition;
		}

		private void OnPlayerStepOn(Entity entity)
		{
			if(entity.Name == "Player")
			{
				m_StepOn = true;

				Entity delivery = Instantiate(m_DeliveryVariants[Random.Int(0, m_DeliveryVariants.Length)], Vector3.Up * 8.0f);
			}
		}

		private void OnPlayerStepOff(Entity entity)
		{
			if (entity.Name == "Player")
			{
				m_StepOn = false;
			}
		}
	}
}
