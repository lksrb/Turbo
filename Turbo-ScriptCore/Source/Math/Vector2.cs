using System.Runtime.InteropServices;

namespace Turbo
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Vector2
	{
		public float x;
		public float y;

		public Vector2(float _x, float _y)
		{
			x = _x;
			y = _y;
		}

		public Vector2(float xy) : this(xy, xy) { }

		public Vector2(Vector3 other) : this(other.x, other.y) { }
		public Vector2(Vector4 other) : this(other.x, other.y) { }

		public float Length
		{
			get => Mathf.Sqrt(Mathf.Dot(this, this));
		}

		public void Normalize()
		{
			this *= 1.0f / Length;
		}

		public override string ToString()
		{
			return $"(x: {x}, y: {y})";
		}
		public static Vector2 Zero => new Vector2(0.0f, 0.0f);
		public static Vector2 Up => new Vector2(0.0f, 1.0f);
		public static Vector2 Right => new Vector2(1.0f, 0.0f);

		public static Vector2 operator +(Vector2 u, Vector2 v)
		{
			return new Vector2(u.x + v.x, u.y + v.y);
		}

		public static Vector2 operator -(Vector2 u, Vector2 v)
		{
			return new Vector2(u.x - v.x, u.y - v.y);
		}

		public static Vector2 operator *(Vector2 u, float v)
		{
			return new Vector2(u.x * v, u.y * v);
		}
	}
}
