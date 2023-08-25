namespace Turbo
{
	public static class Assets
	{
		public static Prefab LoadPrefab(string path)
		{
			ulong id = InternalCalls.Assets_Load_Prefab(path);
			return new Prefab(id);
		}
	}
}
