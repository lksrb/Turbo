using System;

namespace Turbo
{
	public class Entity
	{
		public readonly ulong ID;
		public TransformComponent transform;
		
		protected Entity() { ID = 0; }

		internal Entity(ulong id) 
		{ 
			ID = id;

			transform = GetComponent<TransformComponent>();
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

		public Entity FindEntityByName(string name)
		{
			ulong entity_id = InternalCalls.Entity_FindEntityByName(name);
			if (entity_id == 0)
				return null;

			return new Entity(entity_id);
		}

		public T As<T>() where T : Entity, new()
		{
			object instance = InternalCalls.Entity_Instance_Get(ID);
			return instance as T;
		}


	}
}
