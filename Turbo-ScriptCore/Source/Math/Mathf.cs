using System;

namespace Turbo
{
	public static class Mathf
	{
		public const float PI = 3.1415926535897931f;

		#region General
		public static float Abs(float value)
		{
			return (float)Math.Abs(value);
		}
		public static Vector3 Abs(Vector3 value)
		{
			Vector3 result = value;
			result.x = Mathf.Abs(result.x);
			result.y = Mathf.Abs(result.y);
			result.z = Mathf.Abs(result.z);
			return result;
		}

		public static float Pow(float value, float power = 2.0f)
		{
			return (float)Math.Pow(value, power);
		}
		public static float Sqrt(float value)
		{
			if(value < 0.0f)
			{
				Log.Fatal("Mathf.Sqrt(); value cannot be less than zero!");
				return value;
			}
			return (float)Math.Sqrt(value);
		}
		public static float Radians(float angle)
		{
			return (Mathf.PI / 180.0f) * angle;
		}
		public static float Sign(float value)
		{
			return (float)Math.Sign(value);
		}
		#endregion

		#region Linear Algebra
		public static Vector3 Sign(Vector3 value)
		{
			float x = Mathf.Sign(value.x);
			float y = Mathf.Sign(value.y);
			float z = Mathf.Sign(value.z);
			return new Vector3(x, y, z);
		}
		public static Vector3 Lerp(Vector3 start, Vector3 end, float maxDistanceDelta)
		{
			Vector3 distance = end - start;

			// Removes unnecessary approximation
			if(distance.Length < maxDistanceDelta) 
				return end;

			return start + distance * maxDistanceDelta;
		}
		public static Vector3 Cerp(Vector3 start, Vector3 end, float maxDistanceDelta)
		{
			Vector3 direction = end - start;

			// Removes unnecessary approximation
			if (direction.Length < maxDistanceDelta)
				return end;

			return start + Mathf.Sign(direction) * maxDistanceDelta;
		}

		public static float Dot(Vector2 a, Vector2 b)
		{
			return a.x * b.x + a.y * b.y;
		}
		public static float Dot(Vector3 a, Vector3 b)
		{
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}
		public static float Dot(Vector4 a, Vector4 b)
		{
			return a.x * b.x + a.y * b.y + a.z * b.z + a.w + b.w;
		}
		#endregion

		#region Trigonometric functions
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
		#endregion

		#region Inverse trigonometric functions
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
		#endregion
	}
}
