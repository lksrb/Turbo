using System.Linq.Expressions;

namespace Turbo
{
	// TODO: We can make them constant and be used as default parameters in some math functions
	public static class Frame
	{
		/**
		 * Time difference between current and previous frame
		 */
		public static readonly float TimeStep;

		/**
		 * Fixed time step used by physics engine
		 */
		//public static readonly float FixedTimeStep = 1 / 60.0f; // Maybe I dont need this since physics engine is running at TimeStep
	}
}
