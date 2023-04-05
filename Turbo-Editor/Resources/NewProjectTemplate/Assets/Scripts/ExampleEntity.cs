using Turbo;

namespace ExampleNamespace
{
	public class ExampleEntity : Entity
    {
        void OnStart()
        {
            Log.Info("Hello entity!");
        }

        void OnUpdate(float ts)
        {

        }
    }
}