using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Turbo
{
	public abstract class Component
	{
		public Entity Entity { get; internal set; }
	}

	public class TransformComponent : Component
	{
		public Vector3 Translation
		{
			get
			{
				InternalCalls.Component_Transform_Get_Translation(Entity.ID, out Vector3 translation);
				return translation;
			}
			set
			{
				InternalCalls.Component_Transform_Set_Translation(Entity.ID, ref value);
			}
		}

		public Vector3 Rotation
		{
			get
			{
				InternalCalls.Component_Transform_Get_Rotation(Entity.ID, out Vector3 rotation);
				return rotation;
			}
			set
			{
				InternalCalls.Component_Transform_Set_Rotation(Entity.ID, ref value);
			}
		}

		public Vector3 Scale
		{
			get
			{
				InternalCalls.Component_Transform_Get_Scale(Entity.ID, out Vector3 scale);
				return scale;
			}
			set
			{
				InternalCalls.Component_Transform_Set_Scale(Entity.ID, ref value);
			}
		}
	}

	// Physics
	public class Rigidbody2DComponent : Component
	{
		public bool Gravity
		{
			get
			{
				return InternalCalls.Component_Rigidbody2D_Get_Gravity(Entity.ID);
			}
			set
			{
				InternalCalls.Component_Rigidbody2D_Set_Gravity(Entity.ID, value);
			}
		}
		public void ApplyTorque(float torque, bool wake = true)
		{
			InternalCalls.Component_Rigidbody2D_ApplyTorque(Entity.ID, torque, wake);
		}

		public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPosition, bool wake = true)
		{
			InternalCalls.Component_Rigidbody2D_ApplyLinearImpulse(Entity.ID, ref impulse, ref worldPosition, wake);
		}

		public void ApplyLinearImpulse(Vector2 impulse, bool wake = true)
		{
			InternalCalls.Component_Rigidbody2D_ApplyLinearImpulseToCenter(Entity.ID, ref impulse, wake);
		}
	}
	public class BoxCollider2DComponent : Component
	{
		public Vector2 Offset
		{
			get
			{
				InternalCalls.Component_BoxCollider2D_Get_Offset(Entity.ID, out Vector2 offset);
				return offset;
			}
			set
			{
				InternalCalls.Component_BoxCollider2D_Set_Offset(Entity.ID, ref value);
			}
		}

		public Vector2 Size
		{
			get
			{
				InternalCalls.Component_BoxCollider2D_Get_Size(Entity.ID, out Vector2 size);
				return size;
			}
			set
			{
				InternalCalls.Component_BoxCollider2D_Set_Size(Entity.ID, ref value);
			}
		}

	}
}
