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

		public float Length() => Mathf.Sqrt(Dot(this, this));
		public void Normalize() => this *= 1.0f / Length();

		public override string ToString() => $"Vector3(X: {X}, Y: {Y}, Z: {Z})";

		public static Vector3 Cos(Vector3 vector)
		{
			return new Vector3(Mathf.Cos(vector.X), Mathf.Cos(vector.Y), Mathf.Cos(vector.Z));
		}

		public static Vector3 Sin(Vector3 vector)
		{
			return new Vector3(Mathf.Sin(vector.X), Mathf.Sin(vector.Y), Mathf.Sin(vector.Z));
		}

		public static Vector3 Cross(Vector3 x, Vector3 y)
		{
			return new Vector3(
				x.Y * y.Z - y.Y * x.Z,
				x.Z * y.X - y.Z * x.X,
				x.X * y.Y - y.X * x.Y
			);
		}

		public override bool Equals(object obj) => base.Equals(obj);
		public override int GetHashCode() => base.GetHashCode();

		public static Vector3 Zero => new Vector3(0.0f, 0.0f, 0.0f);
		public static Vector3 Up => new Vector3(0.0f, 1.0f, 0.0f);
		public static Vector3 Down => new Vector3(0.0f, -1.0f, 0.0f);
		public static Vector3 Right => new Vector3(1.0f, 0.0f, 0.0f);
		public static Vector3 Left => new Vector3(-1.0f, 0.0f, 0.0f);
		public static Vector3 Forward => new Vector3(0.0f, 0.0f, -1.0f);
		public static Vector3 Back => new Vector3(0.0f, 0.0f, 1.0f);

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

		public Vector2 YZ
		{
			get => new Vector2(Y, Z);
			set
			{
				Y = value.X;
				Z = value.Y;
			}
		}


		public static bool operator ==(Vector3 u, Vector3 v) => u.X == v.X && u.Y == v.Y && u.Z == v.Z;
		public static bool operator !=(Vector3 u, Vector3 v) => !(u == v);
		public static Vector3 operator +(Vector3 u, Vector3 v) => new Vector3(u.X + v.X, u.Y + v.Y, u.Z + v.Z);
		public static Vector3 operator +(Vector3 u, Vector2 v) => new Vector3(u.X + v.X, u.Y + v.Y, u.Z);

		public static Vector3 operator -(Vector3 u, Vector3 v) => new Vector3(u.X - v.X, u.Y - v.Y, u.Z - v.Z);
		public static Vector3 operator -(Vector3 v) => new Vector3(-v.X, -v.X, -v.Z);

		public static Vector3 operator *(Vector3 u, float v) => new Vector3(u.X * v, u.Y * v, u.Z * v);
		public static Vector3 operator *(float u, Vector3 v) => v * u;

		public static Vector3 operator /(Vector3 u, float v) => new Vector3(u.X / v, u.Y / v, u.Z / v);
		public static Vector3 operator /(float u, Vector3 v) => new Vector3(u / v.X, u / v.Y, u / v.Z);

		// Extensions
		public static Vector3 Abs(Vector3 value)
		{
			Vector3 result = value;
			result.X = Mathf.Abs(result.X);
			result.Y = Mathf.Abs(result.Y);
			result.Z = Mathf.Abs(result.Z);
			return result;
		}

		public static Vector3 Normalize(Vector3 value)
		{
			Vector3 result = value;
			result.Normalize();
			return result;
		}

		public static Vector3 Radians(Vector3 degrees) => degrees * Mathf.Deg2Rad;
		public static Vector3 Degrees(Vector3 radians) => radians * Mathf.Rad2Deg;

		public static Vector3 Clamp(Vector3 value, Vector3 min, Vector3 max)
		{
			Vector3 result = value;
			result.X = Mathf.Clamp(result.X, min.X, max.X);
			result.Y = Mathf.Clamp(result.Y, min.Y, max.Y);
			result.Z = Mathf.Clamp(result.Z, min.Z, max.Z);

			return result;
		}

		public static Vector3 Sign(Vector3 value)
		{
			float x = Mathf.Sign(value.X);
			float y = Mathf.Sign(value.Y);
			float z = Mathf.Sign(value.Z);
			return new Vector3(x, y, z);
		}

		public static Vector3 Lerp(Vector3 start, Vector3 end, float maxDistanceDelta) => start + (end - start) * maxDistanceDelta;
		public static Vector3 Cerp(Vector3 start, Vector3 end, float maxDistanceDelta) => start + Sign(end - start) * maxDistanceDelta;
		public static float Dot(Vector3 a, Vector3 b) => a.X * b.X + a.Y * b.Y + a.Z * b.Z;
	}
}
