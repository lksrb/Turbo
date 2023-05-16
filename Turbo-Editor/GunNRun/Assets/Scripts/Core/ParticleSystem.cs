using System.Collections.Generic;
using Turbo;

namespace GunNRun
{
	public class ParticleSystem : Entity
	{
		private const int m_MaxParticles = 1;
		private List<Entity> m_ParticleEntities = new List<Entity>(m_MaxParticles);

		protected override void OnCreate()
		{
			for (int i = 0; i <  m_MaxParticles; i++)
			{
				Entity entity = Scene.CreateEntity("Particle");
				entity.Transform.Translation = Vector3.Forward;
				var src = entity.AddComponent<SpriteRendererComponent>();
				src.Color = Vector4.Blue;
				m_ParticleEntities.Add(entity);
			}
		}

		protected override void OnUpdate()
		{

		}
	}
}
