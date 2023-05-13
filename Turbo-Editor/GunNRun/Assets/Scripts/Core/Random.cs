namespace Turbo
{
	public class Random
	{
		private static Random s_Instance = new Random();

		private System.Random m_RandomGenerator;

		private Random()
		{
			m_RandomGenerator = new System.Random();

			// First one is usually slow
			m_RandomGenerator.NextDouble();
		}

		public static float Float(float min = 0.0f, float max = 1.0f) => min + (float)s_Instance.m_RandomGenerator.NextDouble() * (max - min);
		public static Vector2 Float2(float min = 0.0f, float max = 1.0f) => new Vector2(Float(min, max), Float(min, max));
		public static Vector3 Float3(float min = 0.0f, float max = 1.0f) => new Vector3(Float(min, max), Float(min, max), Float(min, max));
		public static Vector4 Float4(float min = 0.0f, float max = 1.0f) => new Vector4(Float(min, max), Float(min, max), Float(min, max), Float(min, max));

		public static Vector2 InsideUnitCircle() => Float2(-1.0f, 1.0f);
	}
}
