using System.Runtime.InteropServices;

namespace Turbo
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Quaternion
	{
		public float W;
		public float X;
		public float Y;
		public float Z;

		public Quaternion(Vector3 euler)
		{
			Vector3 c = Vector3.Cos(euler * 0.5f);
			Vector3 s = Vector3.Sin(euler * 0.5f);

			W = c.X * c.Y * c.Z + s.X * s.Y * s.Z;
			X = s.X * c.Y * c.Z - c.X * s.Y * s.Z;
			Y = c.X * s.Y * c.Z + s.X * c.Y * s.Z;
			Z = c.X * c.Y * s.Z - s.X * s.Y * c.Z;
		}

		public Quaternion(float w, float x, float y, float z)
		{
			W = w;
			X = x;
			Y = y;
			Z = z;
		}

		public static Quaternion Identity => new Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
		public static Quaternion Zero => new Quaternion(0.0f, 0.0f, 0.0f, 0.0f);

		public void Normalize()
		{
			float magnitude = Mathf.Sqrt(Dot(this, this));
			W /= magnitude;
			X /= magnitude;
			Y /= magnitude;
			Z /= magnitude;
		}

		public Vector3 GetEulerAngles()
		{
			// Calculate Euler angles
			float x = Mathf.Atan2(2 * (X * Y + W * Z), W * W + X * X - Y * Y - Z * Z);
			float y = Mathf.Asin(-2 * (X * Z - W * Y));
			float z = Mathf.Atan2(2 * (Y * Z + W * X), W * W - X * X - Y * Y + Z * Z);

			return new Vector3(x, y, z);
		}

		public override int GetHashCode() => (W, X, Y, Z).GetHashCode();
		public override bool Equals(object obj) => obj is Quaternion other && Equals(other);
		public bool Equals(Quaternion right) => W == right.W && X == right.X && Y == right.Y && Z == right.Z;

		public override string ToString() => $"Quaternion(W: {W}, X: {X}, Y: {Y}, Z: {Z})";

		public static Vector3 operator *(Quaternion q, Vector3 v)
		{
			Vector3 qv = new Vector3(q.X, q.Y, q.Z);
			Vector3 uv = Vector3.Cross(qv, v);
			Vector3 uuv = Vector3.Cross(qv, uv);
			return v + ((uv * q.W) + uuv) * 2.0f;
		}
		public static bool operator ==(Quaternion left, Quaternion right) => left.Equals(right);
		public static bool operator !=(Quaternion left, Quaternion right) => !(left == right);

		public static Quaternion operator -(Quaternion q) => new Quaternion(-q.W, -q.X, -q.Y, -q.Z);

		// Extension static functions
		public static float Dot(Quaternion q1, Quaternion q2) => q1.W * q2.W + q1.X * q2.X + q1.Y * q2.Y + q1.Z * q2.Z;

		public static Quaternion LookAt(Vector3 forward, Vector3 upwards)
		{
			forward.Normalize();
			upwards.Normalize();

			Vector3 right = Vector3.Cross(upwards, forward);
			Vector3 newUpwards = Vector3.Cross(forward, right);

			float m00 = right.X;
			float m01 = right.Y;
			float m02 = right.Z;
			float m10 = newUpwards.X;
			float m11 = newUpwards.Y;
			float m12 = newUpwards.Z;
			float m20 = forward.X;
			float m21 = forward.Y;
			float m22 = forward.Z;

			float num8 = (m00 + m11) + m22;
			Quaternion quaternion = new Quaternion();
			if (num8 > 0f)
			{
				float num = (float)Mathf.Sqrt(num8 + 1f);
				quaternion.W = num * 0.5f;
				num = 0.5f / num;
				quaternion.X = (m12 - m21) * num;
				quaternion.Y = (m20 - m02) * num;
				quaternion.Z = (m01 - m10) * num;
				return quaternion;
			}
			if ((m00 >= m11) && (m00 >= m22))
			{
				float num7 = (float)Mathf.Sqrt(((1f + m00) - m11) - m22);
				float num4 = 0.5f / num7;
				quaternion.X = 0.5f * num7;
				quaternion.Y = (m01 + m10) * num4;
				quaternion.Z = (m02 + m20) * num4;
				quaternion.W = (m12 - m21) * num4;
				return quaternion;
			}
			if (m11 > m22)
			{
				float num6 = (float)Mathf.Sqrt(((1f + m11) - m00) - m22);
				float num3 = 0.5f / num6;
				quaternion.X = (m10 + m01) * num3;
				quaternion.Y = 0.5f * num6;
				quaternion.Z = (m21 + m12) * num3;
				quaternion.W = (m20 - m02) * num3;
				return quaternion;
			}
			float num5 = (float)Mathf.Sqrt(((1f + m22) - m00) - m11);
			float num2 = 0.5f / num5;
			quaternion.X = (m20 + m02) * num2;
			quaternion.Y = (m21 + m12) * num2;
			quaternion.Z = 0.5f * num5;
			quaternion.W = (m01 - m10) * num2;
			return quaternion;
		}

		// From glm::slerp
		public static Quaternion Slerp(Quaternion start, Quaternion end, float maxRotationDelta)
		{
			// Sadly float.Epsilon is too small 
			const float minDeltaValue = float.Epsilon;

			start.Normalize();
			end.Normalize();

			// Calculate the angle between the quaternions
			float dot = Quaternion.Dot(start, end);

			// if dot < 0, the interpolation will take the long way around the spehere
			// One quat must be negated
			if (dot < 0.0f)
			{
				end = -end;
				dot = -dot;
			}

			Quaternion interpolated = Quaternion.Zero;
			// Quaternions are very close, perform linear interpolation
			if (1.0f - dot > minDeltaValue) // Maybe this constant should be maxRotationDelta?
			{
				float theta = Mathf.Acos(dot);
				// Interpolate using spherical linear interpolation formula
				float sinTheta = Mathf.Sin(theta);
				float weightStart = Mathf.Sin((1.0f - maxRotationDelta) * theta) / sinTheta;
				float weightEnd = Mathf.Sin(maxRotationDelta * theta) / sinTheta;

				interpolated.W = weightStart * start.W + weightEnd * end.W;
				interpolated.X = weightStart * start.X + weightEnd * end.X;
				interpolated.Y = weightStart * start.Y + weightEnd * end.Y;
				interpolated.Z = weightStart * start.Z + weightEnd * end.Z;
			}
			else // Linear interpolation
			{
				interpolated.W = Mathf.Mix(start.W, end.W, maxRotationDelta);
				interpolated.X = Mathf.Mix(start.X, end.X, maxRotationDelta);
				interpolated.Y = Mathf.Mix(start.Y, end.Y, maxRotationDelta);
				interpolated.Z = Mathf.Mix(start.Z, end.Z, maxRotationDelta);
			}

			interpolated.Normalize();

			return interpolated; // Normalize the result
		}
	}
}
