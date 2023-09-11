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

		public bool IsNaN() => float.IsNaN(W) || float.IsNaN(X) || float.IsNaN(Y) || float.IsNaN(Z);

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
		public static Quaternion operator +(Quaternion q1, Quaternion q2) => Normalize(new Quaternion(q1.W + q2.W, q1.X + q2.X, q1.Y + q2.Y, q1.Z + q2.Z));
		public static Quaternion operator -(Quaternion q1, Quaternion q2) => Normalize(new Quaternion(q1.W - q2.W, q1.X - q2.X, q1.Y - q2.Y, q1.Z - q2.Z));
		public static Quaternion operator *(Quaternion q1, Quaternion q2)
		{
			Quaternion result = new Quaternion(
				q1.W * q2.W - q1.X * q2.X - q1.Y * q2.Y - q1.Z * q2.Z, 
				q1.W * q2.X + q1.X * q2.W + q1.Y * q2.Z - q1.Z * q2.Y,
				q1.W * q2.Y - q1.X * q2.Z + q1.Y * q2.W + q1.Z * q2.X,
				q1.W * q2.Z + q1.X * q2.Y - q1.Y * q2.X + q1.Z * q2.W);
			result.Normalize();
			return result;
		}

		public static Quaternion operator -(Quaternion q) => new Quaternion(-q.W, -q.X, -q.Y, -q.Z);

		// Extension static functions
		public static float Dot(Quaternion q1, Quaternion q2) => q1.W * q2.W + q1.X * q2.X + q1.Y * q2.Y + q1.Z * q2.Z;
		public static Quaternion Normalize(Quaternion q) { q.Normalize(); return q; }

		public static Quaternion AxisAngle(Vector3 axis, float angle)
		{
			// Ensure the axis is normalized
			axis = Vector3.Normalize(axis);

			// Calculate half angle
			float halfAngle = angle * 0.5f;
			float sinHalfAngle = Mathf.Sin(halfAngle);

			// Create the quaternion
			return new Quaternion(
				Mathf.Cos(halfAngle),
				axis.X * sinHalfAngle,
				axis.Y * sinHalfAngle,
				axis.Z * sinHalfAngle
			);
		}

		public static Quaternion LookAt(Vector3 source, Vector3 target, Vector3 upwards) => LookAt(target - source, upwards);

		public static Quaternion LookAt(Vector3 forward, Vector3 upwards)
		{
			// Ensure that the vector is normalized
			forward.Normalize();

			// Calculate the rotation quaternion
			float dot = Vector3.Dot(Vector3.Forward, forward);

			if (Mathf.Abs(dot + 1.0f) < 0.000001f)
			{
				// Source and target are exactly opposite, so use the up direction
				return Quaternion.AxisAngle(upwards, Mathf.PI);
			}
			else if (Mathf.Abs(dot - 1.0f) < 0.000001f)
			{
				// Source and target are already aligned
				return Quaternion.Identity;
			}
			else
			{
				// Calculate the rotation axis
				Vector3 rotationAxis = Vector3.Cross(Vector3.Forward, forward);

				// Calculate the rotation angle
				float rotationAngle = Mathf.Acos(dot);

				// Create the quaternion
				return Quaternion.AxisAngle(rotationAxis, rotationAngle);
			}
		}

		// From glm.hpp
		public static Quaternion Slerp(Quaternion start, Quaternion end, float maxRotationDelta)
		{
			start.Normalize();
			end.Normalize();

			// Calculate the angle between the quaternions
			float dot = Dot(start, end);

			// if dot < 0, the interpolation will take the long way around the spehere
			// One quat must be negated
			if (dot < 0.0f)
			{
				end = -end;
				dot = -dot;
			}

			Quaternion result = Quaternion.Zero;

			// Quaternions are very close, perform linear interpolation
			if (1.0f - dot > maxRotationDelta)
			{
				float theta = Mathf.Acos(dot);
				// Interpolate using spherical linear interpolation formula
				float sinTheta = Mathf.Sin(theta);
				float weightStart = Mathf.Sin((1.0f - maxRotationDelta) * theta) / sinTheta;
				float weightEnd = Mathf.Sin(maxRotationDelta * theta) / sinTheta;

				result.W = weightStart * start.W + weightEnd * end.W;
				result.X = weightStart * start.X + weightEnd * end.X;
				result.Y = weightStart * start.Y + weightEnd * end.Y;
				result.Z = weightStart * start.Z + weightEnd * end.Z;
			}
			else // Linear interpolation
			{
				result = Lerp(start, end, maxRotationDelta);
			}

			// Normalize the result
			result.Normalize();

			return result;
		}

		public static Quaternion Lerp(Quaternion start, Quaternion end, float maxRotationDelta)
		{
			Quaternion result = new Quaternion(
				Mathf.Mix(start.W, end.W, maxRotationDelta), 
				Mathf.Mix(start.X, end.X, maxRotationDelta),
				Mathf.Mix(start.Y, end.Y, maxRotationDelta),
				Mathf.Mix(start.Z, end.Z, maxRotationDelta)
			);
			result.Normalize();
			return result;
		}
	}
}
