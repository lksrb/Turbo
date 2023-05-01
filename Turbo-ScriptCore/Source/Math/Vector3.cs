using System.Runtime.InteropServices;

namespace Turbo
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Vector3
	{
		public float X;
		public float Y;
		public float Z;

		public Vector3(float x, float y, float z) { X = x; Y = y; Z = z; }
		public Vector3(Vector3 other) : this(other.X, other.Y, other.Z) { }
		public Vector3(Vector2 xy, float z = 0.0f) : this(xy.X, xy.Y, z) { }
		public Vector3(float scalar) : this(scalar, scalar, scalar) { }

		public float Length
		{
			get => Mathf.Sqrt(Mathf.Dot(this, this));
		}

		public override string ToString()
		{
			return $"(x: {X}, y: {Y}, z: {Z})";
		}

		public void Normalize()
		{
			this *= 1.0f / Length;
		}

		public static Vector3 Zero => new Vector3(0.0f, 0.0f, 0.0f);
		public static Vector3 Up => new Vector3(0.0f, 1.0f, 0.0f);
		public static Vector3 Right => new Vector3(1.0f, 0.0f, 0.0f);
		public static Vector3 Forward => new Vector3(0.0f, 0.0f, 1.0f);

		public Vector2 XY => new Vector2(X, Y);
		public Vector2 XZ => new Vector2(X, Z);
		public Vector2 YZ => new Vector2(Y, Z);

		public static Vector3 operator +(Vector3 u, Vector3 v)
		{
			return new Vector3(u.X + v.X, u.Y + v.Y, u.Z + v.Z);
		}

		public static Vector3 operator -(Vector3 u, Vector3 v)
		{
			return new Vector3(u.X - v.X, u.Y - v.Y, u.Z - v.Z);
		}

		public static Vector3 operator *(Vector3 u, float v)
		{
			return new Vector3(u.X * v, u.Y * v, u.Z * v);
		}
	}
}
