namespace Mystery
{
	internal enum MovementFlags : uint
	{
		Move = 1 << 0,
		Rotate = 1 << 1,
		All = Move | Rotate
	}
}
