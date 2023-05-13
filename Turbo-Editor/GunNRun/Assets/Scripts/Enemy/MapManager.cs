using System.Runtime.InteropServices;
using Turbo;

namespace GunNRun
{
	public class MapManager : Entity
	{
		protected override void OnCreate()
		{
			CollisionFilter filter = new CollisionFilter();
			filter.CollisionCategory = (ushort)GameCategory.Wall;
			filter.CollisionMask = (ushort)GameCategory.Everything;

			var children = GetChildren();
			foreach (var child in children)
			{
				child.GetComponent<BoxCollider2DComponent>().Filter = filter;
			} 
		}

		protected override void OnUpdate()
		{
		}
	}
}
