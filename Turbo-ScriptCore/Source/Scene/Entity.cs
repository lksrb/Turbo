﻿using System;

namespace Turbo
{
	public class Entity
	{
		public readonly ulong ID;
		public TransformComponent Transform;
		public string Name;

		public Action<Entity> OnCollisionBegin2D;
		public Action<Entity> OnCollisionEnd2D;
		public Action<Entity> OnTriggerBegin2D;
		public Action<Entity> OnTriggerEnd2D;

		protected Entity() { ID = 0; }
		protected virtual void OnCreate() {}
		protected virtual void OnUpdate(float ts) {}

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

		private void OnCollisionBegin2D_Internal(ulong id) => OnCollisionBegin2D?.Invoke(new Entity(id));
		private void OnCollisionEnd2D_Internal(ulong id) => OnCollisionEnd2D?.Invoke(new Entity(id));
		private void OnTriggerBegin2D_Internal(ulong id) => OnTriggerBegin2D?.Invoke(new Entity(id));
		private void OnTriggerEnd2D_Internal(ulong id) => OnTriggerEnd2D?.Invoke(new Entity(id));
	}
}
