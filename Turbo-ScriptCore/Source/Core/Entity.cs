using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Turbo
{
	public class Entity
	{
		public readonly ulong ID;
		public Transform transform;
		
		protected Entity() { ID = 0; }

		internal Entity(ulong id) 
		{ 
			ID = id; 
		}
	}
}
