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

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Application_Close();

		#endregion

		#region Assets

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Assets_Load_Prefab(string path);

		#endregion

		#region Debug

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_DrawLine(ref Vector3 start, ref Vector3 end, ref Color color);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_DrawCircle(ref Vector3 position, ref Vector3 rotation, float radius, ref Color color);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void DebugRenderer_DrawBox(ref Vector3 position, ref  Vector3 rotation, ref Vector3 scale, ref Color color);

		#endregion

		#region Logging

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Log_String(LogLevel level, string message);

		#endregion

		#region Input

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(KeyCode code);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyUp(KeyCode code);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonDown(MouseCode code);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonUp(MouseCode code);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Input_SetCursorMode(CursorMode mode);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Input_GetMousePosition(out Vector2 mousePosition);

		#endregion

		#region Scene

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Scene_CreateEntity(ulong parentUUID, string name);

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
		internal extern static void Entity_Remove_Component(ulong uuid, Type componentType);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_FindEntityByName(string name);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Entity_Get_Name(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Entity_Set_Name(ulong uuid, string name);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static Entity[] Entity_Get_Children(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Entity_UnParent(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_InstantiatePrefab(ulong prefabID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_InstantiatePrefabWithTranslation(ulong prefabID, ref Vector3 translation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_InstantiateChildPrefabWithTranslation(ulong uuid, string path, ref Vector3 translation);

		#endregion

		#region Physics2D

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Physics2D_RayCast(Vector2 a, Vector2 b);

		#endregion

		#region Physics

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Physics_CastRay(ref Vector3 start, ref Vector3 direction, RayTarget target, out UnmanagedRayCastResult result);

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

		#region ScriptComponent

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static object Component_Script_Get_Instance(ulong uuid);

		#endregion

		#region LineRendererComponent

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_LineRenderer_Get_Color(ulong uuid, out Color color);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_LineRenderer_Set_Color(ulong uuid, ref Color color);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_LineRenderer_Get_Destination(ulong uuid, out Vector3 position1);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_LineRenderer_Set_Destination(ulong uuid, ref Vector3 position1);

		#endregion

		#region SpriteRendererComponent

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_SpriteRenderer_Get_Color(ulong uuid, out Color color);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_SpriteRenderer_Set_Color(ulong uuid, ref Color color);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_SpriteRenderer_SetSpriteBounds(ulong uuid, Vector2 position, Vector2 size);

		#endregion

		#region TextComponent

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Component_Text_Get_Text(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Text_Set_Text(ulong uuid, string text);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Text_Get_Color(ulong uuid, out Vector4 color);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Text_Set_Color(ulong uuid, ref Vector4 color);

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

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_AudioSource_Play(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_AudioSource_Pause(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_AudioSource_Resume(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_AudioSource_Stop(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Component_AudioSource_IsPlaying(ulong uuid);

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
		internal extern static RigidbodyType Component_Rigidbody2D_Get_BodyType(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody2D_Set_BodyType(ulong uuid, RigidbodyType type);

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

		// IsTrigger
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Component_BoxCollider2D_Get_IsTrigger(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_BoxCollider2D_Set_IsTrigger(ulong uuid, bool isTrigger);

		// CollisionCategory
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_BoxCollider2D_Set_CollisionFilter(ulong uuid, ushort category, ushort mask);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_BoxCollider2D_Get_CollisionFilter(ulong uuid, out ushort category, out ushort mask);
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

		// CollisionCategory
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_CircleCollider2D_Set_CollisionFilter(ulong uuid, ushort category, ushort mask);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_CircleCollider2D_Get_CollisionFilter(ulong uuid, out ushort category, out ushort mask);

		#endregion

		#region RigidbodyComponent

		// Body type
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static RigidbodyType Component_Rigidbody_Get_BodyType(ulong uuid);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody_Set_BodyType(ulong uuid, RigidbodyType type);

		// Linear velocity
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody_Get_LinearVelocity(ulong uuid, out Vector3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody_Set_LinearVelocity(ulong uuid, ref Vector3 velocity);

		// Angular velocity
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody_Get_AngularVelocity(ulong uuid, out Vector3 velocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody_Set_AngularVelocity(ulong uuid, ref Vector3 velocity);

		// Position
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody_Get_Position(ulong uuid, out Vector3 position);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody_Set_Position(ulong uuid, ref Vector3 position);

		// Rotation
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody_Get_Rotation(ulong uuid, out Quaternion rotation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody_Set_Rotation(ulong uuid, ref Quaternion rotation);

		// Rotate
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody_Rotate(ulong uuid, ref Vector3 velocity);

		// Add force
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Component_Rigidbody_AddForce(ulong uuid, ref Vector3 force, ForceMode mode);

		#endregion

		#endregion
	}
}
