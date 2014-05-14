

#include <LTBL/Light/ShadowFin.h>

namespace ltbl
{
	ShadowFin::ShadowFin()
		: m_umbraBrightness(1.0f), m_penumbraBrightness(1.0f)
	{
	}

	ShadowFin::~ShadowFin()
	{
	}

	void ShadowFin::render(float transparency)
	{
		if(m_penumbraBrightness != 1.0f)
		{
			glColor4f(1.0f, 1.0f, 1.0f, m_penumbraBrightness * transparency);

			glBlendFunc(GL_ZERO, GL_SRC_ALPHA);

			glBegin(GL_TRIANGLES);
				glTexCoord2i(0, 1); glVertex2f(m_rootPos.x, m_rootPos.y);
				glTexCoord2i(1, 0); glVertex2f(m_rootPos.x + m_penumbra.x, m_rootPos.y + m_penumbra.y);
				glTexCoord2i(0, 0); glVertex2f(m_rootPos.x + m_umbra.x, m_rootPos.y + m_umbra.y);
			glEnd();
		}
		else
		{
			if(m_umbraBrightness != 1.0f)
			{
				float brightness = (1.0f - m_umbraBrightness) * transparency;
				glColor4f(1.0f, 1.0f, 1.0f, brightness);
			}
			else
				glColor4f(1.0f, 1.0f, 1.0f, transparency);

			glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

			glBegin(GL_TRIANGLES);
				glTexCoord2i(0, 1); glVertex2f(m_rootPos.x, m_rootPos.y);
				glTexCoord2i(0, 0); glVertex2f(m_rootPos.x + m_penumbra.x, m_rootPos.y + m_penumbra.y);
				glTexCoord2i(1, 0); glVertex2f(m_rootPos.x + m_umbra.x, m_rootPos.y + m_umbra.y);
			glEnd();
		}

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
}
