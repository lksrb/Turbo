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
		public Vector4(float scalar) : this(scalar, scalar, scalar, scalar) { }

		public float Length => Mathf.Sqrt(Mathf.Dot(this, this));
		public void Normalize() => this *= 1.0f / Length;

		public override string ToString() => $"(x: {X}, y: {Y}, z: {Z}, w: {W})";

		public override bool Equals(object obj) => base.Equals(obj);
		public override int GetHashCode() => base.GetHashCode();

		public static Vector4 Zero => new Vector4(0.0f);

		// Temporary
		public static Vector4 Red => new Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		public static Vector4 Green => new Vector4(0.0f, 1.0f, 0.0f, 1.0f);
		public static Vector4 Blue => new Vector4(0.0f, 0.0f, 1.0f, 1.0f);

		public Vector2 XY
		{
			get => new Vector2(X, Y);
			set
			{
				X = value.X;
				Y = value.Y;
			}
		}

		public Vector2 XZ
		{
			get => new Vector2(X, Z);
			set
			{
				X = value.X;
				Z = value.Y;
			}
		}

		public Vector2 XW
		{
			get => new Vector2(X, W);
			set
			{
				X = value.X;
				W = value.Y;
			}
		}

		public Vector2 YZ
		{
			get => new Vector2(Y, Z);
			set
			{
				Y = value.X;
				Z = value.Y;
			}
		}

		public Vector2 YW
		{
			get => new Vector2(Y, W);
			set
			{
				Y = value.X;
				W = value.Y;
			}
		}

		public Vector2 ZW
		{
			get => new Vector2(Z, W);
			set
			{
				Z = value.X;
				W = value.Y;
			}
		}

		public Vector3 XYZ
		{
			get => new Vector3(X, Y, Z);
			set
			{
				X = value.X;
				Y = value.Y;
				Z = value.Z;
			}
		}

		public Vector3 YZW
		{
			get => new Vector3(Y, Z, W);
			set
			{
				Y = value.X;
				Z = value.Y;
				W = value.Z;
			}
		}

		public Vector3 XYW
		{
			get => new Vector3(X, Y, W);
			set
			{
				X = value.X;
				Y = value.Y;
				W = value.Z;
			}
		}

		public Vector3 XZW
		{
			get => new Vector3(X, Z, W);
			set
			{
				X = value.X;
				Z = value.Y;
				W = value.Z;
			}
		}

		public static bool operator ==(Vector4 u, Vector4 v) => u.X == v.X && u.Y == v.Y && u.Z == v.Z && u.W == v.W;
		public static bool operator !=(Vector4 u, Vector4 v) => !(u == v);

		public static Vector4 operator +(Vector4 u, Vector4 v) => new Vector4(u.X + v.X, u.Y + v.Y, u.Z + v.Z, u.W + v.W);
		public static Vector4 operator -(Vector4 u, Vector4 v) => new Vector4(u.X - v.X, u.Y - v.Y, u.Z - v.Z, u.W - v.W);
		public static Vector4 operator *(Vector4 u, float v) => new Vector4(u.X * v, u.Y * v, u.Z * v, u.W * v);
		public static Vector4 operator *(float u, Vector4 v) => u * v;
	}
}
