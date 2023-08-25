using System.Collections.Generic;
using Turbo;

namespace Mystery
{
	public class PressurePlate : Entity
	{
		bool m_StepOn = false;

		RigidbodyComponent m_Rigidbody;

		Vector3 m_DefaultPosition;

		Prefab m_DeliveryBoxPrefab;

		float m_DeltaStepOnOff = 0.0f;

		protected override void OnCreate()
		{
			OnTriggerBegin += OnPlayerStepOn;
			OnTriggerEnd += OnPlayerStepOff;

			m_Rigidbody = GetComponent<RigidbodyComponent>();
			m_DefaultPosition = m_Rigidbody.Position;

			m_DeliveryBoxPrefab = Assets.LoadPrefab("Prefabs/DeliveryBox.tprefab");
		}

		protected override void OnUpdate()
		{
			// Minimizing DLL calls
			var currentPosition = m_Rigidbody.Position;

			if (m_StepOn)
			{
				currentPosition.Y = Mathf.Cerp(currentPosition.Y, m_DefaultPosition.Y - Transform.Scale.Y * 0.35f, Frame.TimeStep * 3.0f);
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
				entity.As<Player>().m_BoxesAvailable.Add(Instantiate(m_DeliveryBoxPrefab));
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
