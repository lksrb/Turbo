namespace Turbo
{
	public struct Vector3
	{
		public float x;
		public float y;
		public float z;

		public Vector3(float _x, float _y, float _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}

		public Vector3(Vector3 other) : this(other.x, other.y, other.z) { }

		public Vector3(float xyz) : this(xyz, xyz, xyz) { }

		public float Length
		{
			get => Mathf.Sqrt(Mathf.Dot(this, this));
		}

		public override string ToString()
		{
			return $"(x: {x}, y: {y}, z: {z})";
		}

		public static Vector3 Zero => new Vector3(0.0f, 0.0f, 0.0f);
		public static Vector3 Up => new Vector3(0.0f, 1.0f, 0.0f);
		public static Vector3 Right => new Vector3(1.0f, 0.0f, 0.0f);
		public static Vector3 Forward => new Vector3(0.0f, 0.0f, 1.0f);

		public static Vector3 operator +(Vector3 u, Vector3 v)
		{
			return new Vector3(u.x + v.x, u.y + v.y, u.z + v.z);
		}

		public static Vector3 operator -(Vector3 u, Vector3 v)
		{
			return new Vector3(u.x - v.x, u.y - v.y, u.z - v.z);
		}

		public static Vector3 operator *(Vector3 u, float v)
		{
			return new Vector3(u.x * v, u.y * v, u.z * v);
		}
	}
}
