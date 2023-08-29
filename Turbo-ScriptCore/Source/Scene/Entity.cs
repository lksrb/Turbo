using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace Turbo
{
	public class Entity
	{
		public readonly ulong ID;
		public TransformComponent Transform;

		public string Name
		{
			get => m_Name;
			set
			{
				InternalCalls.Entity_Set_Name(ID, value);
				m_Name = value;
			}
		}

		// Collision 2D callbacks
		protected event Action<Entity> OnCollisionBegin2D;
		protected event Action<Entity> OnCollisionEnd2D;
		protected event Action<Entity> OnTriggerBegin2D;
		protected event Action<Entity> OnTriggerEnd2D;

		// Collision 3D callbacks
		protected event Action<Entity> OnCollisionBegin;
		protected event Action<Entity> OnCollisionEnd;
		protected event Action<Entity> OnTriggerBegin;
		protected event Action<Entity> OnTriggerEnd;

		// Caching components
		private Dictionary<Type, Component> m_CachedComponents = new Dictionary<Type, Component>();
		private string m_Name;

		protected Entity() { ID = 0; }
		protected virtual void OnCreate() { }
		protected virtual void OnUpdate() { }

		internal Entity(ulong id)
		{
			ID = id;

			Transform = GetComponent<TransformComponent>();
			m_Name = InternalCalls.Entity_Get_Name(ID);
		}

		public void UnParent() => InternalCalls.Entity_UnParent(ID);

		public bool HasComponent<T>() where T : Component, new() => InternalCalls.Entity_Has_Component(ID, typeof(T));

		public T GetComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);

			if (!HasComponent<T>())
			{
				if (m_CachedComponents.ContainsKey(componentType))
					m_CachedComponents.Remove(componentType);

				return null;
			}

			if (m_CachedComponents.ContainsKey(componentType))
				return m_CachedComponents[componentType] as T;

			T component = new T() { Entity = this };
			m_CachedComponents.Add(componentType, component);
			return component;
		}

		public T AddComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);

			if (HasComponent<T>())
			{
				Log.Error($"Entity already has this component! {componentType.Name}");
				return null;
			}

			InternalCalls.Entity_Add_Component(ID, componentType);

			T component = new T() { Entity = this };
			m_CachedComponents.Add(componentType, component);
			return component;
		}

		public void RemoveComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);

			if (!HasComponent<T>())
			{
				Log.Error($"Entity doesn't have this component! {componentType.Name}");
				return;
			}

			if (m_CachedComponents.ContainsKey(componentType))
				m_CachedComponents.Remove(componentType);

			// TODO: Check if the component was remoed
			InternalCalls.Entity_Remove_Component(ID, componentType);
		}

		public static bool operator ==(Entity a, Entity b)
		{
			// Check if both objects are null
			if (a is null && b is null)
			{
				return true;
			}

			// Check if either object is null
			if (a is null || b is null)
			{
				return false;
			}

			return a.ID == b.ID;
		}

		public static bool operator !=(Entity a, Entity b) => !(a == b);

		public Entity[] GetChildren() => InternalCalls.Entity_Get_Children(ID);

		public Entity Instantiate(Prefab prefab)
		{
			ulong entityID = InternalCalls.Entity_InstantiatePrefab(prefab.ID);
			if (entityID == 0)
				return null;

			return new Entity(entityID);
		}

		public Entity Instantiate(Prefab prefab, Vector3 translation)
		{
			ulong entityID = InternalCalls.Entity_InstantiatePrefabWithTranslation(prefab.ID, ref translation);
			if (entityID == 0)
				return null;

			return new Entity(entityID);
		}

		// TODO:
		private Entity InstantiateChild(string prefabPath, Vector3 translation)
		{
			ulong entityID = InternalCalls.Entity_InstantiateChildPrefabWithTranslation(ID, prefabPath, ref translation);
			if (entityID == 0)
				return null;

			return new Entity(entityID);
		}

		public Entity FindEntityByName(string name)
		{
			ulong entityID = InternalCalls.Entity_FindEntityByName(name);
			if (entityID == 0)
				return null;

			return new Entity(entityID);
		}

		public bool Is<T>() where T : Entity
		{
			ScriptComponent script = GetComponent<ScriptComponent>();
			object instance = script != null ? script.Instance : null;
			return instance is T;
		}

		public T As<T>() where T : Entity
		{
			ScriptComponent script = GetComponent<ScriptComponent>();
			object instance = script != null ? script.Instance : null;
			return instance is T ? instance as T : null;
		}

		private void OnCollisionBegin2D_Internal(ulong id) => OnCollisionBegin2D?.Invoke(new Entity(id));
		private void OnCollisionEnd2D_Internal(ulong id) => OnCollisionEnd2D?.Invoke(new Entity(id));
		private void OnTriggerBegin2D_Internal(ulong id) => OnTriggerBegin2D?.Invoke(new Entity(id));
		private void OnTriggerEnd2D_Internal(ulong id) => OnTriggerEnd2D?.Invoke(new Entity(id));

		private void OnCollisionBegin_Internal(ulong id) => OnCollisionBegin?.Invoke(new Entity(id));
		private void OnCollisionEnd_Internal(ulong id) => OnCollisionEnd?.Invoke(new Entity(id));
		private void OnTriggerBegin_Internal(ulong id) => OnTriggerBegin?.Invoke(new Entity(id));
		private void OnTriggerEnd_Internal(ulong id) => OnTriggerEnd?.Invoke(new Entity(id));

		public override string ToString() => Name;
		public override bool Equals(object obj) => (obj is Entity) ? (obj as Entity) == this : base.Equals(obj);
		public override int GetHashCode() => (int)ID;
	}
}
