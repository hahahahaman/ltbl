#ifndef LTBL_SHADOWFIN_H
#define LTBL_SHADOWFIN_H

#include <SFML/OpenGL.hpp>
#include <SFML/System/Vector2.hpp>

/**
*The triangular shapes that are used to create shadow effect
*/

namespace ltbl
{
	class ShadowFin
	{
	public:
		sf::Vector2f m_rootPos;
		sf::Vector2f m_umbra;
		sf::Vector2f m_penumbra;

		float m_umbraBrightness;
		float m_penumbraBrightness;

		ShadowFin();
		~ShadowFin();

		void render(float transparency);
	};
}

#endif
