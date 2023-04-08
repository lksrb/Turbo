using System.Runtime.InteropServices;

namespace Turbo
{
	[StructLayout(LayoutKind.Explicit)]
	public struct Vector4
	{
		[FieldOffset(0)] public float x;
		[FieldOffset(4)] public float y;
		[FieldOffset(8)] public float z;
		[FieldOffset(12)] public float w;

		[FieldOffset(0)] public float r;
		[FieldOffset(4)] public float g;
		[FieldOffset(8)] public float b;
		[FieldOffset(12)] public float a;

		public Vector4(float _x, float _y, float _z, float _w)
		{
			r = x = _x;
			g = y = _y;
			b = z = _z;
			a = w = _w;
		}
		public Vector4(float xyzw) : this(xyzw, xyzw, xyzw, xyzw) { }

		public float Length
		{
			get => Mathf.Sqrt(Mathf.Dot(this, this));
		}
		public override string ToString()
		{
			return $"(x: {x}, y: {y}, z: {z}, w: {w})";
		}

		public static Vector4 operator +(Vector4 u, Vector4 v)
		{
			return new Vector4(u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w);
		}

		public static Vector4 operator -(Vector4 u, Vector4 v)
		{
			return new Vector4(u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w);
		}

		public static Vector4 operator *(Vector4 u, float v)
		{
			return new Vector4(u.x * v, u.y * v, u.z * v, u.w * v);
		}
	}
}
