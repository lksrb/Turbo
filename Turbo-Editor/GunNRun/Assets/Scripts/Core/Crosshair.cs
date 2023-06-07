using Turbo;

namespace GunNRun
{
	public class Crosshair : Entity
	{
		protected override void OnUpdate()
		{
			Vector3 m_WorldMousePosition = Camera.ScreenToWorldPosition(Input.MousePosition);

			Vector3 translation = Transform.Translation;
			translation.XY = m_WorldMousePosition;
			Transform.Translation = translation;
		}
	}
}
