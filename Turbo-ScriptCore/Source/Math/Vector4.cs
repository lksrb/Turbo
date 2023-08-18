using System.Runtime.InteropServices;

namespace Turbo
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Vector4
	{
		public float X;
		public float Y;
		public float Z;
		public float W;

		public Vector4(float x, float y, float z, float w)
		{
			X = x;
			Y = y;
			Z = z;
			W = w;
		}

		public Vector4(Vector3 xyz, float w) : this(xyz.X, xyz.Y, xyz.Z, w) { }
		public Vector4(Vector2 xy, float z, float w) : this(xy.X, xy.Y, z, w) { }
		public Vector4(float scalar) : this(scalar, scalar, scalar, scalar) { }

		public float Length() => Mathf.Sqrt(Dot(this, this));
		public void Normalize() => this *= 1.0f / Length();

		public override string ToString() => $"Vector4(X: {X}, Y: {Y}, Z: {Z}, W: {W})";

		public override bool Equals(object obj) => base.Equals(obj);
		public override int GetHashCode() => base.GetHashCode();

		public static Vector4 Zero => new Vector4(0.0f);

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
		public static Vector4 operator *(float u, Vector4 v) => v * u;

		public static implicit operator Vector4(Color value) => new Vector4(value.R, value.G, value.B, value.A);

		// Extensions
		public static Vector4 Clamp(Vector4 value, Vector4 min, Vector4 max)
		{
			Vector4 result = value;
			result.X = Mathf.Clamp(result.X, min.X, max.X);
			result.Y = Mathf.Clamp(result.Y, min.Y, max.Y);
			result.Z = Mathf.Clamp(result.Z, min.Z, max.Z);
			result.W = Mathf.Clamp(result.W, min.W, max.W);

			return result;
		}

		public static float Dot(Vector4 a, Vector4 b) => a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W + b.W;
	}
}
