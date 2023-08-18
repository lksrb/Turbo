using System;

namespace Turbo
{
	public static class Mathf
	{
		#region General

		public const float Infinity = float.PositiveInfinity;
		public const float PI = 3.1415926535897931f;
		public const float HalfPI = 3.1415926535897931f / 2.0f;
		public const float TwoPI = 3.1415926535897931f * 2.0f;

		public const float Rad2Deg = 180.0f / PI;
		public const float Deg2Rad = PI / 180.0f;

		public static float Abs(float value) => (float)Math.Abs(value);
		public static float Normalize(float value, float min, float max) => (value - min) / (max - min);

		public static float Cerp(float start, float end, float maxDistanceDelta)
		{
			float direction = end - start;

			// Removes unnecessary approximation
			if (Normalize(direction, start, end) < maxDistanceDelta)
				return end;

			return start + Sign(direction) * maxDistanceDelta;
		}

		public static float Lerp(float start, float end, float maxDistanceDelta) => start + (end - start) * maxDistanceDelta;

		public static float Mix(float start, float end, float t) => start * (1.0f - t) + end * t;

		public static float Pow(float value, float power = 2.0f) => (float)Math.Pow(value, power);
		public static float Sqrt(float value) => (float)Math.Sqrt(value);

		public static float Radians(float degrees) => degrees * Deg2Rad;
		public static float Degrees(float radians) => radians * Rad2Deg;

		public static float Sign(float value) => (float)Math.Sign(value);

		public static float Max(float value1, float value2) => value1 > value2 ? value1 : value2;
		public static float Min(float value1, float value2) => value1 < value2 ? value1 : value2;

		public static float Clamp(float value, float minValue, float maxValue)
		{
			if (value > maxValue)
				return maxValue;
			if (value < minValue)
				return minValue;

			return value;
		}

		public static float Clamp01(float value) => Clamp(value, 0, 1);

		public static float DeltaAngle(float current, float target)
		{
			float delta = target - current;
			if (delta > 180f)
			{
				delta -= 360f;
			}
			else if (delta < -180f)
			{
				delta += 360f;
			}
			return delta;
		}

		public static float SmoothDamp(float start, float end, ref float currentVelocity, float smoothTime, float maxSpeed, float deltaTime)
		{
			// Based on Game Programming Gems 4 Chapter 1.10
			smoothTime = Max(0.0001F, smoothTime);
			float omega = 2F / smoothTime;

			float x = omega * deltaTime;
			float exp = 1F / (1F + x + 0.48F * x * x + 0.235F * x * x * x);
			float change = start - end;
			float originalTo = end;

			// Clamp maximum speed
			float maxChange = maxSpeed * smoothTime;
			change = Clamp(change, -maxChange, maxChange);
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

		public static float SmoothDampAngle(float current, float target, ref float currentVelocity, float smoothTime, float maxSpeed = Mathf.Infinity, float deltaTime = 1.0f)
		{
			smoothTime = Max(0.0001f, smoothTime);
			float omega = 2.0f / smoothTime;

			float x = omega * deltaTime;
			float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);

			float diff = DeltaAngle(current, target);
			float change = diff * exp;

			float originalTo = target - diff;
			float original = current;

			float maxChange = maxSpeed * smoothTime;

			change = Clamp(change, -maxChange, maxChange);
			target = originalTo - change;

			float temp = (currentVelocity + omega * change) * deltaTime;
			currentVelocity = (currentVelocity - omega * temp) * exp;

			float output = target + (change + temp) * exp;

			// Ensure the difference between the output angle and target angle is less than 180 degrees
			if (Abs(output - target) > 180f)
			{
				output = target + 360f - output;
			}

			return output;
		}


		#endregion

		#region Trigonometric functions

		public static float Sin(float radians) => (float)Math.Sin(radians);
		public static float Cos(float radians) => (float)Math.Cos(radians);
		public static float Tan(float radians) => (float)Math.Tan(radians);
		public static float Sinh(float radians) => (float)Math.Sinh(radians);
		public static float Cosh(float radians) => (float)Math.Cosh(radians);
		public static float Tanh(float radians) => (float)Math.Tanh(radians);

		#endregion

		#region Inverse trigonometric functions

		public static float Asin(float value) => (float)Math.Asin(value);
		public static float Acos(float value) => (float)Math.Acos(value);
		public static float Atan(float value) => (float)Math.Atan(value);
		public static float Atan2(float y, float x) => (float)Math.Atan2(y, x);

		#endregion
	}
}
