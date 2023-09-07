using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Turbo
{
	public struct CastRayResult
	{
		public Vector3 HitPosition;
		public Entity HitEntity;
	}

	public struct CastRayAllResult
	{
		internal CastRayResult[] HitResults;

		public CastRayResult this[int index] => HitResults[index];

		public IEnumerator GetEnumerator() => HitResults.GetEnumerator();
	}

	public enum RayTarget : uint
	{
		Nearest = 0,
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
	[StructLayout(LayoutKind.Sequential)]
	internal struct InternalCastRayResult
	{
		internal Vector3 HitPosition;
		internal ulong HitEntity;
	}

	public static class Physics
	{
		public static bool CastRay(Vector3 origin, Vector3 direction, float length, RayTarget target, out CastRayResult result)
		{
			// Ensure vector normalization
			direction.Normalize();

			InternalCalls.Physics_CastRay(ref origin, ref direction, length, target, out InternalCastRayResult outResult);

			// Hit
			if (outResult.HitEntity != 0)
			{
				result.HitEntity = new Entity(outResult.HitEntity);
				result.HitPosition = outResult.HitPosition;
				return true;
			}

			result.HitEntity = null;
			result.HitPosition = new Vector3(Mathf.Infinity);

			return false;
		}

		// Note that this is expensive since we are creating new entities
		public static bool CastRayAll(Vector3 origin, Vector3 direction, float length, out CastRayAllResult result)
		{
			direction.Normalize();

			InternalCastRayResult[] results = InternalCalls.Physics_CastRayAll(ref origin, ref direction, length);

			if (results != null)
			{
				result.HitResults = new CastRayResult[results.Length];

				// Create all entities
				for (int i = 0; i < results.Length; i++)
				{
					result.HitResults[i].HitEntity = new Entity(results[i].HitEntity);
					result.HitResults[i].HitPosition = results[i].HitPosition;
				}

				return true;
			}

			result.HitResults = null;

			return false;
		}
	}
}
