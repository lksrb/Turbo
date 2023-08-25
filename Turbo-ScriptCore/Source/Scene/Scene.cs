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

		// Queues up entity for deletion
		// Deletion will happen in post update
		public static void DestroyEntity(Entity entity)
		{
			InternalCalls.Scene_DestroyEntity(entity.ID);
		}

	}
}
