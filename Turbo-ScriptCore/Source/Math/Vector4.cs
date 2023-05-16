using System.Runtime.InteropServices;

namespace Turbo
{
	[StructLayout(LayoutKind.Explicit)]
	public struct Vector4
	{
		[FieldOffset(0)] public float X;
		[FieldOffset(4)] public float Y;
		[FieldOffset(8)] public float Z;
		[FieldOffset(12)] public float W;

		[FieldOffset(0)] public float R;
		[FieldOffset(4)] public float G;
		[FieldOffset(8)] public float B;
		[FieldOffset(12)] public float A;

		public Vector4(float x, float y, float z, float w)
		{
			R = X = x;
			G = Y = y;
			B = Z = z;
			A = W = w;
		}

		public Vector4(Vector3 xyz, float w) : this(xyz.X, xyz.Y, xyz.Z, w) { }
		public Vector4(Vector2 xy, float z, float w) : this(xy.X, xy.Y, z, w) { }
		public Vector4(float xyzw) : this(xyzw, xyzw, xyzw, xyzw) { }

		public float Length => Mathf.Sqrt(Mathf.Dot(this, this));
		public void Normalize() => this *= 1.0f / Length;

		public override string ToString() => $"(x: {X}, y: {Y}, z: {Z}, w: {W})";

		public static Vector4 Zero => new Vector4(0.0f);

		public static Vector4 Red => new Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		public static Vector4 Green => new Vector4(0.0f, 1.0f, 0.0f, 1.0f);
		public static Vector4 Blue => new Vector4(0.0f, 0.0f, 1.0f, 1.0f);

		public static Vector4 operator +(Vector4 u, Vector4 v) => new Vector4(u.X + v.X, u.Y + v.Y, u.Z + v.Z, u.W + v.W);
		public static Vector4 operator -(Vector4 u, Vector4 v) => new Vector4(u.X - v.X, u.Y - v.Y, u.Z - v.Z, u.W - v.W);
		public static Vector4 operator *(Vector4 u, float v) => new Vector4(u.X * v, u.Y * v, u.Z * v, u.W * v);
		public static Vector4 operator *(float u, Vector4 v) => u * v;
	}
}
