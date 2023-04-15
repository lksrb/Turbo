using Turbo;

namespace ExampleNamespace
{
	public class ExampleEntity : Entity
    {
        protected override void OnCreate()
        {
            Log.Info("Hello entity!");
        }

        protected override void OnUpdate(float ts)
        {

        }
    }
}