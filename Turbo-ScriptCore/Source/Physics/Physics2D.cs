namespace Turbo
{
	public struct RayCast2D
	{
		public Entity Result;
		public bool Hit;

		public RayCast2D(Entity result, bool hit)
		{
			Result = result;
			Hit = hit;
		}
	}

	public static class Physics2D
	{
		// Simplified version of a raycast, good for now
		public static RayCast2D RayCast(Vector2 a, Vector2 b)
		{
			ulong id = InternalCalls.Physics2D_RayCast(a, b);

			RayCast2D rayCast = new RayCast2D(null, false);

			if (id != 0)
			{
				rayCast.Result = new Entity(id);
				rayCast.Hit = true;
			}

			return rayCast;
		}
	}
}
