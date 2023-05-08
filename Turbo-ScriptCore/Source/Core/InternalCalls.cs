using System;
using System.Runtime.CompilerServices;

namespace Turbo
{
	internal static class InternalCalls
	{
		#region Application

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static uint Application_GetWidth();

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static uint Application_GetHeight();

		#endregion

		#region Logging

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Log_String(LogLevel level, string message);

		#endregion

		#region Input

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyPressed(KeyCode code);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyReleased(KeyCode code);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonPressed(MouseCode code);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonReleased(MouseCode code);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static int Input_GetMouseX();
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static int Input_GetMouseY();

		#endregion

		#region Scene

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Scene_CreateEntity(string name);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Scene_DestroyEntity(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Scene_ScreenToWorldPosition(Vector2 screenPosition, out Vector3 worldPosition);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Scene_WorldToScreenPosition(Vector3 worldPosition, out Vector2 screenPosition);

		#endregion

		#region Entity

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_Has_Component(ulong uuid, Type componentType);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Entity_Add_Component(ulong uuid, Type componentType);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_FindEntityByName(string name);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static object Entity_Get_Instance(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Entity_Get_Name(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static Entity[] Entity_Get_Children(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_InstantiatePrefabWithTranslation(string path, ref Vector3 translation);

		#endregion

		#region Physics2D

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Physics2D_RayCast(Vector2 a, Vector2 b);

		#endregion

		#region Components 

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

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_SpriteRenderer_SetSpriteBounds(ulong uuid, Vector2 position, Vector2 size);

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

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody2D_Set_LinearVelocity(ulong uuid, ref Vector2 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody2D_Get_LinearVelocity(ulong uuid, out Vector2 velocity);

		// Gravity
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Component_Rigidbody2D_Get_GravityScale(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody2D_Set_GravityScale(ulong uuid, float gravityScale);

		// BodyType
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static Rigidbody2DComponent.BodyType Component_Rigidbody2D_Get_BodyType(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody2D_Set_BodyType(ulong uuid, Rigidbody2DComponent.BodyType type);

		// Is Enabled
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody2D_Set_Enabled(ulong uuid, bool enabled);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Component_Rigidbody2D_Get_Enabled(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody2D_Set_ContactEnabled(ulong uuid, bool enabled);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Component_Rigidbody2D_Get_ContactEnabled(ulong uuid);

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

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_BoxCollider2D_Set_IsSensor(ulong uuid, bool isSensor);

		// CollisionCategory
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_BoxCollider2D_Set_CollisionCategory(ulong uuid, ushort category);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ushort Component_BoxCollider2D_Get_CollisionCategory(ulong uuid);
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

		#endregion
	}
}
