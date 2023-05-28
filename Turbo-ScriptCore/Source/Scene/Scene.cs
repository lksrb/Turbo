namespace Turbo
{
	public static class Scene
	{
		public static Entity CreateEntity(string name)
		{
			ulong id = InternalCalls.Scene_CreateEntity(0, name);
			return new Entity(id);
		}

		public static Entity CreateChildEntity(Entity parent, string name)
		{
			ulong id = InternalCalls.Scene_CreateEntity(parent.ID, name);
			return new Entity(id);
		}

		public static void DestroyEntity(Entity entity)
		{
			InternalCalls.Scene_DestroyEntity(entity.ID);
		}

		// TODO: Think about where this goes
		// FIXME: Now duplicated (in Entity.cs)
		public static Entity InstantiateEntity(string prefabPath, Vector3 translation)
		{
			ulong entityID = InternalCalls.Entity_InstantiatePrefabWithTranslation(prefabPath, ref translation);
			if (entityID == 0)
				return null;

			return new Entity(entityID);
		}
	}
}
