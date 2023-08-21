using System.Runtime.InteropServices;

namespace Turbo
{
	public struct Ray
	{
		public Vector3 Start;
		public Vector3 Direction; // Is also a length of the direction vector

		public Ray(Vector3 start, Vector3 direction)
		{
			Start = start;
			Direction = direction;
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct RayCastResult
	{
		public Vector3 HitPosition;
		public Entity HitEntity;
	}

	public enum RayTarget : uint
	{
		Closest = 0,
		Furthest,
		Any
	}

	public enum RigidbodyType : uint
	{
		Static = 0,
		Kinematic,
		Dynamic
	}

	public enum ForceMode : uint
	{
		Force = 0,
		Impulse
	}

	// Matches C++ side
	[StructLayout(LayoutKind.Sequential, Pack = 16)]
	internal struct UnmanagedRayCastResult
	{
		internal Vector3 HitPosition;
		internal ulong HitEntity;

		internal bool Hit() { return HitEntity != 0; }
	}

	public static class Physics
	{
		public static bool CastRay(Ray ray, RayTarget target, out RayCastResult result)
		{
			InternalCalls.Physics_CastRay(ref ray.Start, ref ray.Direction, target, out UnmanagedRayCastResult unmanagedResult);

			// Hit
			if (unmanagedResult.Hit())
			{
				result.HitEntity = new Entity(unmanagedResult.HitEntity);
				result.HitPosition = unmanagedResult.HitPosition;
				return true;
			}

			result.HitEntity = null;
			result.HitPosition = Vector3.Zero;

			return false;
		}
	}
}
