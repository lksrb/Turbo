using System;

namespace Turbo
{
	public static class Mathf
	{
		#region General

		public const float PI = 3.1415926535897931f;

		public static float Abs(float value)
		{
			return (float)Math.Abs(value);
		}
		public static Vector3 Abs(Vector3 value)
		{
			Vector3 result = value;
			result.X = Mathf.Abs(result.X);
			result.Y = Mathf.Abs(result.Y);
			result.Z = Mathf.Abs(result.Z);
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

		public static float Max(float value1, float value2)
		{
			if (value1 < value2)
				return value2;

			return value1;
		}

		public static float Min(float value1, float value2)
		{
			if (value1 < value2)
				return value1;

			return value2;
		}

		public static float Clamp(float value, float minValue, float maxValue)
		{
			if (value > maxValue)
				return maxValue;
			if (value < minValue)
				return minValue;

			return value;
		}

		public static float SmoothDamp(float start, float end, ref float currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
		{
			// Based on Game Programming Gems 4 Chapter 1.10
			smoothTime = Mathf.Max(0.0001F, smoothTime);
			float omega = 2F / smoothTime;

			float x = omega * deltaTime;
			float exp = 1F / (1F + x + 0.48F * x * x + 0.235F * x * x * x);
			float change = start - end;
			float originalTo = end;

			// Clamp maximum speed
			float maxChange = maxSpeed * smoothTime;
			change = Mathf.Clamp(change, -maxChange, maxChange);
			end = start - change;

			float temp = (currentVelocity + omega * change) * deltaTime;
			currentVelocity = (currentVelocity - omega * temp) * exp;
			float output = end + (change + temp) * exp;

			// Prevent overshooting
			if (originalTo - start > 0.0F == output > originalTo)
			{
				output = originalTo;
				currentVelocity = (output - originalTo) / deltaTime;
			}

			return output;
		}

		#endregion

		#region Linear Algebra
		public static Vector2 Sign(Vector2 value)
		{
			float x = Mathf.Sign(value.X);
			float y = Mathf.Sign(value.Y);
			return new Vector2(x, y);
		}
		public static Vector3 Sign(Vector3 value)
		{
			float x = Mathf.Sign(value.X);
			float y = Mathf.Sign(value.Y);
			float z = Mathf.Sign(value.Z);
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
		public static Vector2 Lerp(Vector2 start, Vector2 end, float maxDistanceDelta)
		{
			Vector2 distance = end - start;

			// Removes unnecessary approximation
			if (distance.Length < maxDistanceDelta)
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
			return a.X * b.X + a.Y * b.Y;
		}
		public static float Dot(Vector3 a, Vector3 b)
		{
			return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
		}
		public static float Dot(Vector4 a, Vector4 b)
		{
			return a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W + b.W;
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
