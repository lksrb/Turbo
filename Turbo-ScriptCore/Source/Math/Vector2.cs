using System.Runtime.InteropServices;

namespace Turbo
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Vector2
	{
		public float X;
		public float Y;

		public Vector2(float x, float y) { X = x; Y = y; }
		public Vector2(float scalar) : this(scalar, scalar) { }
		public Vector2(Vector3 other) : this(other.X, other.Y) { }
		public Vector2(Vector4 other) : this(other.X, other.Y) { }

		public float Length() => Mathf.Sqrt(Dot(this, this));
		public void Normalize() => this *= 1.0f / Length();

		public override string ToString() => $"Vector2(X: {X}, Y: {Y})";

		public override bool Equals(object obj) => base.Equals(obj);
		public override int GetHashCode() => base.GetHashCode();

		public static Vector2 Zero => new Vector2(0.0f, 0.0f);
		public static Vector2 Up => new Vector2(0.0f, 1.0f);
		public static Vector2 Right => new Vector2(1.0f, 0.0f);

		public static bool operator ==(Vector2 u, Vector2 v) => u.X == v.X && u.Y == v.Y;
		public static bool operator !=(Vector2 u, Vector2 v) => !(u == v);

		public static implicit operator Vector2(Vector3 value) => new Vector2(value);
		public static implicit operator Vector2(Vector4 value) => new Vector2(value);

		public static Vector2 operator +(Vector2 u, Vector2 v) => new Vector2(u.X + v.X, u.Y + v.Y);
		public static Vector2 operator +(Vector2 u, float v) => new Vector2(u.X + v, u.Y + v);

		public static Vector2 operator -(Vector2 u, Vector2 v) => new Vector2(u.X - v.X, u.Y - v.Y);
		public static Vector2 operator -(Vector2 u, float v) => new Vector2(u.X - v, u.Y - v);

		public static Vector2 operator *(Vector2 u, float v) => new Vector2(u.X * v, u.Y * v);
		public static Vector2 operator *(float u, Vector2 v) => v * u;

		public static Vector2 operator -(Vector2 v) => new Vector2(-v.X, -v.X);

		// Extensions

		public static Vector2 Abs(Vector2 value)
		{
			Vector2 result = value;
			result.X = Mathf.Abs(result.X);
			result.Y = Mathf.Abs(result.Y);
			return result;
		}

		public static Vector2 Normalize(Vector2 value)
		{
			Vector2 result = value;
			result.Normalize();
			return result;
		}

		public static Vector2 Radians(Vector2 degrees) => degrees * Mathf.Deg2Rad;
		public static Vector2 Degrees(Vector2 radians) => radians * Mathf.Rad2Deg;

		public static Vector2 Clamp(Vector2 value, Vector2 min, Vector2 max)
		{
			Vector2 result = value;
			result.X = Mathf.Clamp(result.X, min.X, max.X);
			result.Y = Mathf.Clamp(result.Y, min.Y, max.Y);

			return result;
		}

		public static Vector2 Sign(Vector2 value)
		{
			float x = Mathf.Sign(value.X);
			float y = Mathf.Sign(value.Y);
			return new Vector2(x, y);
		}

		public static Vector2 Lerp(Vector2 start, Vector2 end, float maxDistanceDelta) => start + (end - start) * maxDistanceDelta;
		public static float Dot(Vector2 a, Vector2 b) => a.X * b.X + a.Y * b.Y;
	}
}
