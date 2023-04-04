using System;

namespace Turbo
{
	public static class Mathf
	{
		public const float PI = 3.1415926535897931f;

		public static float Radians(float angle)
		{
			return (Mathf.PI / 180.0f) * angle;
		}

		// Trigonometric functions
		public static float Sin(float radians)
		{
			return (float)Math.Sin(radians);
		}
		public static float Cos(float radians)
		{
			return (float)Math.Cos(radians);
		}
		public static float Tan(float radians)
		{
			return (float)Math.Tan(radians);
		}
		public static float Sinh(float radians)
		{
			return (float)Math.Sinh(radians);
		}
		public static float Cosh(float radians)
		{
			return (float)Math.Cosh(radians);
		}

		public static float Tanh(float radians)
		{
			return (float)Math.Tanh(radians);
		}

		// Inverse trigonometric functions
		public static float Asin(float value)
		{
			return (float)Math.Asin(value);
		}
		public static float Acos(float value)
		{
			return (float)Math.Acos(value);
		}
		public static float Atan(float value)
		{
			return (float)Math.Atan(value);
		}
	}
}
