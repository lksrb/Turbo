namespace Turbo
{
	public struct AABB
	{
		Vector3 Min, Max, Size;

		public AABB(Vector3 min, Vector3 max, Vector3 size)
		{
			Min = min;
			Max = max;
			Size = size;
		}

		public AABB(Vector3 center, Vector3 size)
		{
			Min = center - size;
			Max = center + size;
			Size = size;
		}

		public bool Contains(Vector3 point)
		{
			return point.X >= Min.X && point.X <= Max.X &&
				   point.Y >= Min.Y && point.Y <= Max.Y &&
				   point.Z >= Min.Z && point.Z <= Max.Z;
		}
	}
}
