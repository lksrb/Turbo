using System.Collections.Generic;
using Turbo;

namespace GunNRun
{
	public class ParticleSystem : Entity
	{
		private class Particle
		{
			private Entity Handle;
			private Vector2 Velocity;
			private float RotationVelocity;
			private Vector4 Color;
			private SpriteRendererComponent Sprite;
			private bool Shown;

			internal Particle(Entity handle, Vector4 color, Vector2 velocity, float rotationVelocity)
			{
				Handle = handle;
				Velocity = velocity;
				RotationVelocity = rotationVelocity;
				Color = color;
				Sprite = Handle.AddComponent<SpriteRendererComponent>();
				Sprite.SpriteColor = Vector4.Zero;
				Shown = false;
			}

			internal void OnUpdate(float ts)
			{
				if (!Shown)
					return;

				Vector3 translation = Handle.Transform.Translation;
				Vector3 rotation = Handle.Transform.Rotation;
				translation.XY += Velocity * ts;
				rotation.Z += RotationVelocity * ts;
				Handle.Transform.Translation = translation;
				Handle.Transform.Rotation = rotation;
			}

			internal void Hide()
			{
				if (!Shown)
					return;

				Color color = Sprite.SpriteColor;
				color.A = 0.0f;
				Sprite.SpriteColor = color;

				Shown = false;
			}

			internal void Show()
			{
				if (Shown)
					return;

				Sprite.SpriteColor = Color;

				Shown = true;
			}

			internal void SetStartPosition(Vector2 startPosition)
			{
				Vector3 pos = Handle.Transform.Translation;
				pos.XY = startPosition;
				pos.Z = 1.0f;
				Handle.Transform.Translation = pos;
			}
		}

		internal class Builder
		{
			private Vector2 m_DurationRange = new Vector2(0.0f, 1.0f);
			private Vector2 m_VelocityRange = new Vector2(0.0f, 1.0f);
			private Vector2 m_RotationVelocityRange = new Vector2(0.0f, 1.0f);
			private Vector2 m_Scale = new Vector2(1.0f, 1.0f);
			private List<Color> m_ColorRange = new List<Color>();
			private string Name = "ParticleSystem";

			internal Builder SetName(string name)
			{
				Name = name;
				return this;
			}

			internal Builder AddColorRange(List<Vector4> range)
			{
				foreach (var color in range)
				{
					m_ColorRange.Add(color);
				}
				return this;
			}

			internal Builder AddColor(Color color)
			{
				m_ColorRange.Add(color);
				return this;
			}

			internal Builder SetDurationRange(float min, float max)
			{
				m_DurationRange = new Vector2(min, max);
				return this;
			}

			internal Builder SetVelocityRange(float min, float max)
			{
				m_VelocityRange = new Vector2(min, max);
				return this;
			}

			internal Builder SetRotationVelocityRange(float min, float max)
			{
				m_RotationVelocityRange = new Vector2(min, max);
				return this;
			}

			internal Builder SetScale(float x, float y)
			{
				m_Scale = new Vector2(x, y);
				return this;
			}

			internal ParticleSystem Build(int particleAmount, bool autoDestroy = true)
			{
				if (m_ColorRange.Count == 0)
					AddColor(Color.Blue);

				ParticleSystem particleSystem = Scene.InstantiateEntity("Assets/Prefabs/ParticleSystem.tprefab", Vector3.Zero).As<ParticleSystem>();
				particleSystem.Init(Name, particleAmount, m_Scale, m_ColorRange, m_DurationRange, m_VelocityRange, m_RotationVelocityRange, autoDestroy);
				return particleSystem;
			}
		}

		private int m_MaxParticles;
		private List<Particle> m_Particles;
		private List<SingleTickTimer> m_DeathTimers;
		private Vector2 m_DurationRange;
		private List<Color> m_ColorRange;
		private bool m_AutoDestroy;

		private bool m_Start = false;

		private Vector4 RandomColorFromRange()
		{
			int randomColorIndex = Random.Int(0, m_ColorRange.Count);
			return m_ColorRange[randomColorIndex];
		}

		private void Init(string name, int particleAmount, Vector2 scale, List<Color> colorRange, Vector2 durationRange, Vector2 velocityRange, Vector2 rotationVelocityRange, bool autoDestroy)
		{
			m_MaxParticles = particleAmount;
			m_ColorRange = colorRange;
			m_DurationRange = durationRange;
			m_AutoDestroy = autoDestroy;

			m_Particles = new List<Particle>(m_MaxParticles);
			m_DeathTimers = new List<SingleTickTimer>(m_MaxParticles);

			// Set name
			Name = name;

			for (int i = 0; i < m_MaxParticles; i++)
			{
				Entity entity = Scene.CreateChildEntity(this, "Particle");
				entity.Transform.Scale = new Vector3(scale, 1.0f);

				Vector2 randomDirection = Random.InsideUnitCircle();
				randomDirection.Normalize();
				randomDirection *= Mathf.Abs(Random.Float(velocityRange.X, velocityRange.Y));
				Vector4 color = RandomColorFromRange();
				float rotationVelocity = Random.Float(rotationVelocityRange.Y, rotationVelocityRange.Y);
				m_Particles.Add(new Particle(entity, color, randomDirection, rotationVelocity));
			}

			for (int i = 0; i < m_MaxParticles; i++)
			{
				m_DeathTimers.Add(new SingleTickTimer(Random.Float(m_DurationRange.X, m_DurationRange.Y)));
			}
		}

		protected override void OnUpdate()
		{
			if (!m_Start)
				return;

			for (int i = 0; i < m_Particles.Count; i++)
			{
				m_Particles[i].OnUpdate(Frame.TimeStep);
			}

			for (int i = 0; i < m_DeathTimers.Count; i++)
			{
				if (m_DeathTimers[i])
				{
					Particle particle = m_Particles[i];
					particle.Hide();
					m_Particles.RemoveAt(i);
					m_DeathTimers.RemoveAt(i);
				}
			}

			if (m_AutoDestroy && m_Particles.Count == 0)
			{
				Scene.DestroyEntity(this);
			}
		}

		internal void Start(Vector3 position)
		{
			if (m_Start)
				return;

			m_Start = true;

			foreach (var particle in m_Particles)
			{
				particle.SetStartPosition(position);
				particle.Show();
			}
		}
		internal void Reset()
		{
			if (!m_Start)
				return;

			foreach (var particle in m_Particles)
			{
				particle.Hide();
			}
		}

		internal static ParticleSystem.Builder Setup() => new ParticleSystem.Builder();

	}
}
