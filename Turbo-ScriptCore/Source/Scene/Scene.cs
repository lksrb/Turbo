namespace Turbo
{
	public static class Scene
	{
		public static Entity CreateEntity(string name)
		{
			ulong id = InternalCalls.Scene_CreateEntity(name);
			return new Entity(id);
		}

		public static void DestroyEntity(Entity entity)
		{
			InternalCalls.Scene_DestroyEntity(entity.ID);
		}
	}
}
