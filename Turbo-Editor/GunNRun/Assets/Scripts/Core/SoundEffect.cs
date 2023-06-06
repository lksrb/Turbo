using Turbo;

namespace GunNRun
{
	internal enum Effect
	{
		EnemyDeath = 0,
		PickHP,
		PickAmmo,
		Shotgun,
		Sniper,
		Count
	}

	internal static class SoundEffect
	{
		private static readonly string[] s_SoundEffectPaths = new string[(int)Effect.Count]
		{
			"Assets/Prefabs/ShooterDeathSoundEffect.tprefab",
			"Assets/Prefabs/PickUpHPSoundEffect.tprefab",
			"Assets/Prefabs/PickUpAmmoSoundEffect.tprefab",
			"Assets/Prefabs/ShotgunSoundEffect.tprefab",
			"Assets/Prefabs/SniperSoundEffect.tprefab",
		};

		internal static void Play(Effect effect, Vector3 translation)
		{
			if ((int)effect >= (int)Effect.Count)
			{
				Log.Error("Invalid effect!");
				return;
			}

			string effectPath = s_SoundEffectPaths[(int)effect];
			Scene.InstantiateEntity(effectPath, translation);
		}
	}
}
