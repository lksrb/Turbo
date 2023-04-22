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

		// TODO: Think about where to put this
		public static Vector3 ScreenToWorldPosition(Vector2 screenPosition)
		{
			InternalCalls.Scene_ScreenToWorldPosition(screenPosition, out Vector3 worldPosition);
			return worldPosition;
		}
	}
}
