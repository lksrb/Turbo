namespace Mystery
{
	internal enum EnemyState : uint
	{
		ChaseBall = 0,
		GrabAndThrowBall = 1,
		ExhaustedByThrow = 2,
		RunAway = 3,
		Defend = 4,
		Count = 5
	}

	internal interface IEnemyState
	{
		void Enter();
		void OnUpdate();
	}
}
