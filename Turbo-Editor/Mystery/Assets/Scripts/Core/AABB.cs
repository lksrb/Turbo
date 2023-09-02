using Turbo;

namespace Mystery
{
	public struct AABB
	{
		public Vector3 Min, Max, Size;

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

		public Vector3 Center
		{
			get => (Min + Max) / 2.0f;
			set
			{
				Min = value - Size;
				Max = value + Size;
			}
		}

		public bool Contains(Vector3 point)
		{
			return point.X >= Min.X && point.X <= Max.X &&
				   point.Y >= Min.Y && point.Y <= Max.Y &&
				   point.Z >= Min.Z && point.Z <= Max.Z;
		}

		public bool Contains(AABB aabb)
		{
			bool xOverlap = (2.0f * Mathf.Abs(Center.X - aabb.Center.X) < (Size.X + aabb.Size.X));
			bool yOverlap = (2.0f * Mathf.Abs(Center.Y - aabb.Center.Y) < (Size.Y + aabb.Size.Y));
			bool zOverlap = (2.0f * Mathf.Abs(Center.Z - aabb.Center.Z) < (Size.Z + aabb.Size.Z));

			// If there's overlap along all three axes, there's a collision
			return xOverlap && yOverlap && zOverlap;
		}

		public static AABB Zero => new AABB(Vector3.Zero, Vector3.Zero, Vector3.Zero);
	}
}
