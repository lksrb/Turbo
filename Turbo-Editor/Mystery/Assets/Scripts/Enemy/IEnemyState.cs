namespace Mystery
{
	internal enum EnemyState : uint
	{
		ChaseBall = 0,
		Attack = 1,
		ExhaustedByAttack = 2,
		RunAway = 3,
		Defend = 4,
		TryCatchBall = 5,
		Count = 6
	}

	internal interface IEnemyState
	{
		void Enter();
		void OnUpdate();
		void OnPlayerEvent(PlayerEvent playerEvent);
	}
}
