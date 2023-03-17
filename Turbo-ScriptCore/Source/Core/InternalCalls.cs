using System;
using System.Runtime.CompilerServices;

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
		//                                  Input                                   
		// =============================================================================

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyPressed(KeyCode code);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyReleased(KeyCode code);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonPressed(MouseCode code);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonReleased(MouseCode code);

		// =============================================================================
		//                                  Entity                                   
		// =============================================================================

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_Has_Component(ulong id, Type componentType);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_FindEntityByName(string name);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static object Entity_Instance_Get(ulong id);

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
