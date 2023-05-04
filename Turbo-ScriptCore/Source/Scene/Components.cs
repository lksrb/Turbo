﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;

namespace Turbo
{
	public abstract class Component
	{
		public Entity Entity { get; internal set; }
	}

	public class TransformComponent : Component
	{
		public Vector3 Translation
		{
			get
			{
				InternalCalls.Component_Transform_Get_Translation(Entity.ID, out Vector3 translation);
				return translation;
			}
			set
			{
				InternalCalls.Component_Transform_Set_Translation(Entity.ID, ref value);
			}
		}

		public Vector3 Rotation
		{
			get
			{
				InternalCalls.Component_Transform_Get_Rotation(Entity.ID, out Vector3 rotation);
				return rotation;
			}
			set
			{
				InternalCalls.Component_Transform_Set_Rotation(Entity.ID, ref value);
			}
		}

		public Vector3 Scale
		{
			get
			{
				InternalCalls.Component_Transform_Get_Scale(Entity.ID, out Vector3 scale);
				return scale;
			}
			set
			{
				InternalCalls.Component_Transform_Set_Scale(Entity.ID, ref value);
			}
		}
	}

	public class TextComponent : Component
	{
		public string Text
		{
			get => InternalCalls.Component_Text_Get_Text(Entity.ID);
			set => InternalCalls.Component_Text_Set_Text(Entity.ID, value);
		}
	}

	// Audio
	public class AudioSourceComponent : Component
	{
		public float Gain
		{
			get => InternalCalls.Component_AudioSource_Get_Gain(Entity.ID);
			set => InternalCalls.Component_AudioSource_Set_Gain(Entity.ID, value);
		}

		public bool PlayOnStart
		{
			get => InternalCalls.Component_AudioSource_Get_PlayOnStart(Entity.ID);
			set => InternalCalls.Component_AudioSource_Set_PlayOnStart(Entity.ID, value);
		}

		// Can onlybe changed in OnStart
		public bool Loop
		{
			get => InternalCalls.Component_AudioSource_Get_Loop(Entity.ID);
			set => InternalCalls.Component_AudioSource_Set_Loop(Entity.ID, value);
		}
	}

	public class AudioListenerComponent : Component
	{
		public bool IsPrimary
		{
			get => InternalCalls.Component_AudioListener_Get_IsPrimary(Entity.ID);
			set => InternalCalls.Component_AudioListener_Set_IsPrimary(Entity.ID, value);
		}
	}

	// Physics
	public class Rigidbody2DComponent : Component
	{
		public enum BodyType : uint { Static = 0, Dynamic, Kinematic };

		public float GravityScale
		{
			get => InternalCalls.Component_Rigidbody2D_Get_GravityScale(Entity.ID);
			set => InternalCalls.Component_Rigidbody2D_Set_GravityScale(Entity.ID, value);
		}

		public BodyType Type
		{
			get => InternalCalls.Component_Rigidbody2D_Get_BodyType(Entity.ID);
			set => InternalCalls.Component_Rigidbody2D_Set_BodyType(Entity.ID, value);
		}

		public Vector2 Velocity
		{
			get
			{
				InternalCalls.Component_Rigidbody2D_Get_LinearVelocity(Entity.ID, out Vector2 velocity);
				return velocity;
			}
			set
			{
				InternalCalls.Component_Rigidbody2D_Set_LinearVelocity(Entity.ID, ref value);
			}
		}

		// For disabling contact
		public bool IsContactEnabled
		{
			get => InternalCalls.Component_Rigidbody2D_Get_ContactEnabled(Entity.ID);
			set => InternalCalls.Component_Rigidbody2D_Set_ContactEnabled(Entity.ID, value);
		}

		// Disable rigidbody
		public bool IsEnabled
		{
			get => InternalCalls.Component_Rigidbody2D_Get_Enabled(Entity.ID);
			set => InternalCalls.Component_Rigidbody2D_Set_Enabled(Entity.ID, value);
		}

		public void ApplyTorque(float torque, bool wake = true)
		{
			InternalCalls.Component_Rigidbody2D_ApplyTorque(Entity.ID, torque, wake);
		}

		public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPosition, bool wake = true)
		{
			InternalCalls.Component_Rigidbody2D_ApplyLinearImpulse(Entity.ID, ref impulse, ref worldPosition, wake);
		}

		public void ApplyLinearImpulse(Vector2 impulse, bool wake = true)
		{
			InternalCalls.Component_Rigidbody2D_ApplyLinearImpulseToCenter(Entity.ID, ref impulse, wake);
		}

		public void ApplyForceToCenter(Vector2 force, bool wake = true)
		{
			InternalCalls.Component_Rigidbody2D_ApplyLinearImpulseToCenter(Entity.ID, ref force, wake);
		}
	}

	public class BoxCollider2DComponent : Component
	{
		private Vector2 Offset // TODO: Make this more robust
		{
			get
			{
				InternalCalls.Component_BoxCollider2D_Get_Offset(Entity.ID, out Vector2 offset);
				return offset;
			}
			set
			{
				InternalCalls.Component_BoxCollider2D_Set_Offset(Entity.ID, ref value);
			}
		}

		private Vector2 Size // TODO: Make this more robust
		{
			get
			{
				InternalCalls.Component_BoxCollider2D_Get_Size(Entity.ID, out Vector2 size);
				return size;
			}
			set
			{
				InternalCalls.Component_BoxCollider2D_Set_Size(Entity.ID, ref value);
			}
		}

		public ushort CollisionCategory
		{
			get => InternalCalls.Component_BoxCollider2D_Get_CollisionCategory(Entity.ID);
			set => InternalCalls.Component_BoxCollider2D_Set_CollisionCategory(Entity.ID, value);
		}

		public bool IsSensor
		{
			get => InternalCalls.Component_BoxCollider2D_Get_IsSensor(Entity.ID);
		}
	}

	public class CircleCollider2DComponent : Component
	{
		public Vector2 Offset
		{
			get
			{
				InternalCalls.Component_CircleCollider2D_Get_Offset(Entity.ID, out Vector2 offset);
				return offset;
			}
			set
			{
				InternalCalls.Component_CircleCollider2D_Set_Offset(Entity.ID, ref value);
			}
		}

		public float Radius
		{
			get => InternalCalls.Component_CircleCollider2D_Get_Radius(Entity.ID);
			set => InternalCalls.Component_CircleCollider2D_Set_Radius(Entity.ID, ref value);
		}
	}

	public enum ProjectionType : uint
	{
		Orthographics = 0,
		Perspective
	}

	public class CameraComponent : Component
	{
		public ProjectionType Projection
		{
			get => ProjectionType.Orthographics; // TODO:
		}
	}

	public class SpriteRendererComponent : Component
	{
		public Vector4 Color
		{
			get
			{
				InternalCalls.Component_SpriteRenderer_Get_Color(Entity.ID, out Vector4 color);
				return color;
			}
			set
			{
				InternalCalls.Component_SpriteRenderer_Set_Color(Entity.ID, ref value);
			}
		}

		public void SetSpriteBounds(Vector2 position, Vector2 size)
		{
			InternalCalls.Component_SpriteRenderer_SetSpriteBounds(Entity.ID, position, size);
		}
	}
}
