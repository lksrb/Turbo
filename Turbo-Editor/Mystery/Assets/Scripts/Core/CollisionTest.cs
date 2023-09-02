using Turbo;

namespace Mystery
{
	public static class CollisionTest
	{
		public static bool IsInCircle(Vector3 circlePos, Vector3 pos, float radius) => (circlePos.XZ - pos.XZ).Length() < radius;

		public static bool IsInAABB(Vector3 point, Vector3 min, Vector3 max)
		{
			return point.X >= min.X && point.X <= max.X &&
			 point.Y >= min.Y && point.Y <= max.Y &&
			 point.Z >= min.Z && point.Z <= max.Z;
		}
	}
}
