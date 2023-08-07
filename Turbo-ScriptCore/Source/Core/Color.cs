using System.Runtime.InteropServices;

namespace Turbo
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Color
	{
		public float R;
		public float G;
		public float B;
		public float A;

		public Color(float r, float g, float b)
		{
			R = r;
			G = g;
			B = b;
			A = 1.0f;
		}

		public Color(float r, float g, float b, float a)
		{
			R = r;
			G = g;
			B = b;
			A = a;
		}

		public Color(Vector4 color) : this(color.X, color.Y, color.Z, color.W) { }
		public Color(float scalar) : this(scalar, scalar, scalar, scalar) { }

		public override string ToString() => $"Color (r: {R}, g: {G}, b: {B}, a: {A})";
		public override bool Equals(object obj) => base.Equals(obj); // TODO:
		public override int GetHashCode() => base.GetHashCode();

		public Vector2 RG
		{
			get => new Vector2(R, G);
			set
			{
				R = value.X;
				G = value.Y;
			}
		}

		public Vector2 RB
		{
			get => new Vector2(R, B);
			set
			{
				R = value.X;
				B = value.Y;
			}
		}

		public Vector2 RA
		{
			get => new Vector2(R, A);
			set
			{
				R = value.X;
				A = value.Y;
			}
		}

		public Vector2 GB
		{
			get => new Vector2(G, B);
			set
			{
				G = value.X;
				B = value.Y;
			}
		}

		public Vector2 GA
		{
			get => new Vector2(G, A);
			set
			{
				G = value.X;
				A = value.Y;
			}
		}

		public Vector2 BA
		{
			get => new Vector2(B, A);
			set
			{
				B = value.X;
				A = value.Y;
			}
		}

		public Vector3 RGB
		{
			get => new Vector3(R, G, B);
			set
			{
				R = value.X;
				G = value.Y;
				B = value.Z;
			}
		}

		public Vector3 GBA
		{
			get => new Vector3(G, B, A);
			set
			{
				G = value.X;
				B = value.Y;
				A = value.Z;
			}
		}

		public Vector3 RGA
		{
			get => new Vector3(R, G, A);
			set
			{
				R = value.X;
				G = value.Y;
				A = value.Z;
			}
		}

		public Vector3 RBA
		{
			get => new Vector3(R, B, A);
			set
			{
				R = value.X;
				B = value.Y;
				A = value.Z;
			}
		}

		public float this[uint index]
		{
			get
			{
				switch (index)
				{
					case 0: return R;
					case 1: return G;
					case 2: return B;
					case 3: return A;
				}

				Log.Error("Indexing outside of the color!");
				return default;
			}
			set
			{
				if (index > 3)
				{
					Log.Error("Indexing outside of the color!");
					return;
				}

				switch (index)
				{
					case 0: R = value; break;
					case 1: G = value; break;
					case 2: B = value; break;
					case 3: A = value; break;
				}
			}
		}

		// FIXME: "u" or "v" might be null
		public static bool operator ==(Color u, Color v) => u.R == v.R && u.G == v.G && u.B == v.B && u.A == v.A;
		public static bool operator !=(Color u, Color v) => !(u == v);

		public static Color Red => new Color(1.0f, 0.0f, 0.0f, 1.0f);
		public static Color Green => new Color(0.0f, 1.0f, 0.0f, 1.0f);
		public static Color Blue => new Color(0.0f, 0.0f, 1.0f, 1.0f);

		public static Color Black => new Color(0.0f, 0.0f, 0.0f, 1.0f);
		public static Color White => new Color(1.0f);
		public static Color Clear => new Color(0.0f);

		public static Color Orange => new Color(1.0f, 0.5f, 0.0f, 1.0f);
		public static Color Cyan => new Color(0.0f, 1.0f, 1.0f, 1.0f);
		public static Color Gray => new Color(0.5f, 0.5f, 0.5f, 1.0f);
		public static Color Magenta => new Color(1.0f, 0.0f, 1.0f, 1.0f);
		public static Color Yellow => new Color(1.0f, 1.0f, 0.0f, 1.0f);

		public static Color operator +(Color u, Color v) => new Color(u.R + v.R, u.G + v.G, u.B + v.B, u.A + v.A);
		public static Color operator -(Color u, Color v) => new Color(u.R - v.R, u.G - v.G, u.B - v.B, u.A - v.A);
		public static Color operator *(Color u, float v) => new Color(u.R * v, u.G * v, u.B * v, u.A * v);
		public static Color operator *(float u, Color v) => v * u;

		public static Color operator /(Color u, float v)
		{
			if (v == 0.0f)
			{
				Log.Error("Cannot divide by 0!");
				return Color.Clear;
			}

			return u * (1.0f / v);
		}

		public static implicit operator Color(Vector4 value) => new Color(value);
	}
}
