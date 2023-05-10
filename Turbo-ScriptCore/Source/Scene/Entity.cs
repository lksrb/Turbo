using System;

namespace Turbo
{
	public class Entity
	{
		public readonly ulong ID;
		public TransformComponent Transform;
		public string Name;

		// Collision callbacks
		protected event Action<Entity> OnCollisionBegin2D;
		protected event Action<Entity> OnCollisionEnd2D;
		protected event Action<Entity> OnTriggerBegin2D;
		protected event Action<Entity> OnTriggerEnd2D;

		protected Entity() { ID = 0; }
		protected virtual void OnCreate() { }
		protected virtual void OnUpdate(float ts) { }

		internal Entity(ulong id)
		{
			ID = id;

			Transform = GetComponent<TransformComponent>();
			Name = InternalCalls.Entity_Get_Name(ID);
		}

		public bool HasComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);

			return InternalCalls.Entity_Has_Component(ID, componentType);
		}

		public T GetComponent<T>() where T : Component, new()
		{
			if (HasComponent<T>() == false)
			{
				return null;
			}

			T component = new T() { Entity = this };
			return component;
		}

		public T AddComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);

			if (HasComponent<T>() == true)
			{
				Log.Error($"Entity already has this component! {componentType.Name}");
				return null;
			}

			InternalCalls.Entity_Add_Component(ID, componentType);

			T component = new T() { Entity = this };
			return component;
		}

		public static bool operator ==(Entity a, Entity b) => a.ID == b.ID;
		public static bool operator !=(Entity a, Entity b) => !(a == b);

		public Entity[] GetChildren() => InternalCalls.Entity_Get_Children(ID);

		public Entity Instantiate(string prefabPath, Vector3 translation)
		{
			ulong entityID = InternalCalls.Entity_InstantiatePrefabWithTranslation(prefabPath, ref translation);
			if (entityID == 0)
				return null;

			return new Entity(entityID);
		}

		public Entity InstantiateChild(string prefabPath, Vector3 translation)
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

		public T As<T>() where T : Entity, new()
		{
			object instance = InternalCalls.Entity_Get_Instance(ID);
			return instance as T;
		}

		private void OnCollisionBegin2D_Internal(ulong id) => OnCollisionBegin2D?.Invoke(new Entity(id));
		private void OnCollisionEnd2D_Internal(ulong id) => OnCollisionEnd2D?.Invoke(new Entity(id));
		private void OnTriggerBegin2D_Internal(ulong id) => OnTriggerBegin2D?.Invoke(new Entity(id));
		private void OnTriggerEnd2D_Internal(ulong id) => OnTriggerEnd2D?.Invoke(new Entity(id));

		public override string ToString() => Name;
		public override bool Equals(object obj) => base.Equals(obj);
		public override int GetHashCode() => base.GetHashCode();
	}
}
