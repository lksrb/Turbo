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

		#region SpriteRendererComponent

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_SpriteRenderer_Get_Color(ulong uuid, out Vector4 color);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_SpriteRenderer_Set_Color(ulong uuid, ref Vector4 color);

		#endregion

		#region TextComponent

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Component_Text_Get_Text(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Text_Set_Text(ulong uuid, string text);

		#endregion

		#region AudioSourceComponent

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Component_AudioSource_Get_Gain(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Component_AudioSource_Set_Gain(ulong uuid, float gain);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Component_AudioSource_Get_PlayOnStart(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Component_AudioSource_Set_PlayOnStart(ulong uuid, bool playOnStart);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Component_AudioSource_Get_Loop(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Component_AudioSource_Set_Loop(ulong uuid, bool loop);

		#endregion

		#region AudioListenerComponent

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Component_AudioListener_Get_IsPrimary(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Component_AudioListener_Set_IsPrimary(ulong uuid, bool isPrimary);

		#endregion

		#region Rigidbody2DComponent

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody2D_ApplyLinearImpulse(ulong uuid, ref Vector2 worldPosition, ref Vector2 impulse, bool wake);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody2D_ApplyLinearImpulseToCenter(ulong uuid, ref Vector2 impulse, bool wake);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody2D_ApplyForceToCenter(ulong uuid, ref Vector2 force, bool wake);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody2D_ApplyTorque(ulong uuid, float torque, bool wake);

		// Gravity
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Component_Rigidbody2D_Get_Gravity(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody2D_Set_Gravity(ulong uuid, bool gravity);

		// BodyType
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static Rigidbody2DComponent.BodyType Component_Rigidbody2D_Get_BodyType(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody2D_Set_BodyType(ulong uuid, Rigidbody2DComponent.BodyType type);

		#endregion

		#region BoxCollider2DComponent
		// Offset
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_BoxCollider2D_Get_Offset(ulong uuid, out Vector2 offset);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_BoxCollider2D_Set_Offset(ulong uuid, ref Vector2 offset);

		// Size
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_BoxCollider2D_Get_Size(ulong uuid, out Vector2 size);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_BoxCollider2D_Set_Size(ulong uuid, ref Vector2 size);

		// IsSensor
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Component_BoxCollider2D_Get_IsSensor(ulong uuid);
		#endregion

		#region CircleCollider2DComponent
		// Offset
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_CircleCollider2D_Get_Offset(ulong uuid, out Vector2 offset);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_CircleCollider2D_Set_Offset(ulong uuid, ref Vector2 offset);

		// Size
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Component_CircleCollider2D_Get_Radius(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_CircleCollider2D_Set_Radius(ulong uuid, ref float radius);
		#endregion
	}
}
