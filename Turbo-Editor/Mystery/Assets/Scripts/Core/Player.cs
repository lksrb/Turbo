using System.Collections.Generic;
using Turbo;

namespace Mystery
{
	internal class Player : Entity
	{
		public float LinearVelocityMagnifier = 0.0f;
		public float AngularVelocityMagnifier = 0.0f;
		public float PickLength = 0.0f;
		public float PickHeight = 0.0f;

		internal PlayerInput m_Input;
		internal PlayerMovement m_Movement;

		private Entity m_TargetCursor;

		internal List<Entity> m_AvailableDeliveries = new List<Entity>();

		Entity m_Hat;

		Entity m_PickedItem;
		Entity m_PickPlaceHolder;

		protected override void OnCreate()
		{
			m_Input = new PlayerInput();
			m_Movement = new PlayerMovement(this);
			m_Movement.RayCastHit += OnRayCastHit;

			m_Hat = FindEntityByName("Hat");
			m_TargetCursor = FindEntityByName("MOBA_Crosshair");
			m_PickPlaceHolder = FindEntityByName("PickPlaceHolder");

			Log.Info("Hello entity!");
		}

		protected override void OnUpdate()
		{
			m_Input.Update();
			m_Movement.Update();

			OnPickUpUpdate();

			// Hat
			var r = m_Hat.Transform.Rotation;
			r.XY -= Frame.TimeStep;
			m_Hat.Transform.Rotation = r;
		}

		bool IsInCircle(Vector3 a, Vector3 b, float radius)
		{
			return (b - a).Length() < radius;
		}

		void OnPickUpUpdate()
		{
			Vector3 forward = new Quaternion(Transform.Rotation) * Vector3.Forward;
			forward.Y = 0.0f;
			forward.Normalize();

			if (m_PickedItem == null && m_Input.IsPickUpButtonDown)
			{
				foreach (var box in m_AvailableDeliveries)
				{
					if (IsInCircle(box.Transform.Translation, Transform.Translation, 4.0f))
					{
						m_PickedItem = box;
						break;
					}
				}
			}

			if (m_PickedItem != null)
			{
				var rb = m_PickedItem.GetComponent<RigidbodyComponent>();
				rb.Position = m_Movement.Rigidbody.Position + forward * PickLength + Vector3.Up * PickHeight;
				rb.Rotation = m_Movement.Rigidbody.Rotation;
				rb.LinearVelocity = Vector3.Zero;
				rb.AngularVelocity = Vector3.Zero;

				if (Input.IsKeyUp(KeyCode.F))
				{
					if (m_PickedItem.Name == "BouncyBall")
						rb.AddForce(forward * 50.0f, ForceMode.Impulse);

					m_PickedItem = null;
				}
			}
		}

		void OnRayCastHit(Vector3 hitPosition)
		{
			var transform = m_TargetCursor.Transform;
			transform.Translation = hitPosition + Vector3.Up * 0.03f;
			transform.Rotation = Vector3.Right * Mathf.Radians(90.0f);
		}

		internal void OnPickupCollisionBegin(Entity entity)
		{
			Log.Info(entity.Name);

			switch(entity.Name)
			{
				case "DeliveryBox":
				case "DeliveryCapsule":
				case "BouncyBall":
					m_AvailableDeliveries.Add(entity);
					break;
			}
		}

		internal void OnPickupCollisionEnd(Entity entity)
		{
			/*if(m_DeliveryAvailable.Contains(entity))
			{
				m_DeliveryAvailable.Remove(entity);
			}*/
		}

	}
}
