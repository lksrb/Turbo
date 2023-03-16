using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace Turbo
{
	internal static class InternalCalls
	{
// =============================================================================
//                                  Logging                                   
// =============================================================================

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Log_String(LogLevel level, string message);

// =============================================================================
//                                  Components                                   
// =============================================================================
	#region TransformComponent
	// Translation
	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	internal extern static void Component_Transform_Get_Translation(ulong uuid, out Vector3 translation);

	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	internal extern static void Component_Transform_Set_Translation(ulong uuid, ref Vector3 translation);
	// Rotation
	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	internal extern static void Component_Transform_Get_Rotation(ulong uuid, out Vector3 rotation);

	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	internal extern static void Component_Transform_Set_Rotation(ulong uuid, ref Vector3 rotation);
	// Scale
	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	internal extern static void Component_Transform_Get_Scale(ulong uuid, out Vector3 scale);

	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	internal extern static void Component_Transform_Set_Scale(ulong uuid, ref Vector3 scale);
	#endregion
	}
}
