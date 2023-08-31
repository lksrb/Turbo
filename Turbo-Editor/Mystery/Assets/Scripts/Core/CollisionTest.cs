using Turbo;

namespace Mystery
{
	public static class CollisionTest
	{
		public static bool IsInCircle(Vector3 circlePos, Vector3 pos, float radius) => (circlePos.XZ - pos.XZ).Length() < radius;
	}
}
