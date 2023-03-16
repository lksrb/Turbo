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

	public class Transform : Component
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
}
