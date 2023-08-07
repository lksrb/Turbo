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

		public static Vector3 operator *(Quaternion q, Vector3 v)
		{
			Vector3 qv = new Vector3(q.X, q.Y, q.Z);
			Vector3 uv = Vector3.Cross(qv, v);
			Vector3 uuv = Vector3.Cross(qv, uv);
			return v + ((uv * q.W) + uuv) * 2.0f;
		}

		public override int GetHashCode() => (W, X, Y, Z).GetHashCode();

		public override bool Equals(object obj) => obj is Quaternion other && Equals(other);

		public bool Equals(Quaternion right) => W == right.W && X == right.X && Y == right.Y && Z == right.Z;

		public static bool operator ==(Quaternion left, Quaternion right) => left.Equals(right);
		public static bool operator !=(Quaternion left, Quaternion right) => !(left == right);

	}
}
