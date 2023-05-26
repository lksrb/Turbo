using System.Collections.Generic;
using Turbo;

namespace GunNRun
{
	internal class ParticleSystem
	{
		private struct Particle
		{
			private Entity Handle;
			private Vector2 Velocity;
			private float RotationVelocity;

			internal Particle(Entity handle, Vector2 velocity, float rotationVelocity)
			{
				Handle = handle;
				Velocity = velocity;
				RotationVelocity = rotationVelocity;
			}

			internal void OnUpdate(float ts)
			{
				Vector3 translation = Handle.Transform.Translation;
				Vector3 rotation = Handle.Transform.Rotation;
				translation.XY += Velocity * ts;
				rotation.Z += RotationVelocity * ts;
				Handle.Transform.Translation = translation;
				Handle.Transform.Rotation = rotation;
			}

			internal void Kill()
			{
				Scene.DestroyEntity(Handle);
			}
		}

		internal class Builder
		{
			private Vector2 m_DurationRange = new Vector2(0.0f, 1.0f);
			private Vector4 m_ColorRange = Vector4.Blue;
			private Entity m_Parent;

			internal Builder SetColorRange(Vector4 range)
			{
				m_ColorRange = range;
				return this;
			}

			internal Builder SetDurationRange(float min, float max)
			{
				m_DurationRange = new Vector2(min, max);
				return this;
			}

			internal ParticleSystem Build(Entity parent,  int particleAmount)
			{
				ParticleSystem particleSystem = new ParticleSystem(parent);
				particleSystem.SetColorRange(Vector4.Red);
				particleSystem.SetDurationRange(m_DurationRange.X,m_DurationRange.Y);
				particleSystem.Init(particleAmount);
				return particleSystem;
			}
		}

		private int m_MaxParticles;
		private List<Particle> m_Particles;
		private List<SingleUseTimer> m_DeathTimers;

		private Vector2 m_DurationRange = new Vector2(0.0f, 1.0f);
		private Vector4 m_ColorRange = Vector4.Blue;
		private Entity m_Parent;

		internal void Init(int amount)
		{
			m_MaxParticles = amount;
			m_Particles = new List<Particle>(m_MaxParticles);
			m_DeathTimers = new List<SingleUseTimer>(m_MaxParticles);

			Vector3 translation = m_Parent.Transform.Translation;
			for (int i = 0; i < m_MaxParticles; i++)
			{
				Entity entity = Scene.CreateChildEntity(m_Parent, "Particle");
				entity.Transform.Scale *= 0.3f;
				entity.Transform.Translation = new Vector3(translation.XY, 1.0f);
				var src = entity.AddComponent<SpriteRendererComponent>();
				src.Color = m_ColorRange;

				Vector2 randomDirection = Random.InsideUnitCircle();
				randomDirection.Normalize();
				randomDirection *= Random.Float(5, 10);

				m_Particles.Add(new Particle(entity, randomDirection, Random.Float(-10.0f,10.0f)));
			}

			for (int i = 0; i < m_MaxParticles; i++)
			{
				m_DeathTimers.Add(new SingleUseTimer(Random.Float(m_DurationRange.X, m_DurationRange.Y)));
			}
		}

		internal ParticleSystem(Entity parent)
		{
			m_Parent = parent;
		}

		internal void OnUpdate()
		{
			for (int i = 0; i < m_Particles.Count; i++)
			{ 
				m_Particles[i].OnUpdate(Frame.TimeStep);
			}

			for (int i = 0; i < m_DeathTimers.Count; i++)
			{
				if (m_DeathTimers[i])
				{
					Particle particle = m_Particles[i];
					particle.Kill();
					m_Particles.RemoveAt(i);
					m_DeathTimers.RemoveAt(i);
				}
			}
		}

		internal void SetColorRange(Vector4 range)
		{
			m_ColorRange = range;
		}

		internal void SetDurationRange(float min, float max)
		{
			m_DurationRange = new Vector2(min, max);
		}

		internal static ParticleSystem.Builder Setup() => new ParticleSystem.Builder();

	}
}
