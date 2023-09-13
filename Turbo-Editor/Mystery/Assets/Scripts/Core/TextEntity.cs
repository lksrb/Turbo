using Turbo;

namespace Mystery
{
	internal class TextEntity : Entity
	{
		TextComponent m_TextComponent;

		protected override void OnCreate()
		{
			m_TextComponent = GetComponent<TextComponent>();
			m_TextComponent.Text = "...Hello?";
		}

		protected override void OnUpdate()
		{
		}
	}
}
