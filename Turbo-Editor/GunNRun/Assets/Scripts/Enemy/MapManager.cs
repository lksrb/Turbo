using System.Runtime.InteropServices;
using Turbo;

namespace GunNRun
{
	public class MapManager : Entity
	{
		protected override void OnCreate()
		{
			CollisionFilter filter = new CollisionFilter();
			filter.CollisionCategory = (ushort)EntityCategory.Wall;
			filter.CollisionMask = (ushort)EntityCategory.Everything;

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
