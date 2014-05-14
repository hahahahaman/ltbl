

#ifndef LTBL_EMISSIVELIGHT_H
#define LTBL_EMISSIVELIGHT_H

#include <LTBL/QuadTree/QuadTree.h>

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <LTBL/Utils.h>

/**
*"Glowing" lights, the alpha of the texture changes the intensity of glow
*
*Does not emit light!
*/

namespace ltbl
{
	class EmissiveLight : public qdt::QuadTreeOccupant
	{
	private:
		sf::Texture* m_texture;

		float m_angleDegs;

		sf::Vector2f m_halfRenderDims;

		float m_intensity;

        sf::Vector3f m_color;

	public:
		EmissiveLight();

		void render();

        //safe if you use c++11 ptr or manually delete
		void setTexture(sf::Texture* texture);

		void setCenter(const sf::Vector2f &newCenter);
		void incCenter(const sf::Vector2f &increment);
		sf::Vector2f getCenter();

		void setDims(const sf::Vector2f &newScale);
		sf::Vector2f getDims();

		void setRotation(float angleDegs);
		void incRotation(float increment);
        float getRotation();

        void setIntensity(float intensity);
		void incIntensity(float increment);
		float getIntensity() const;

		void setColor(sf::Vector3f color);
		sf::Vector3f getColor() const;
	};
}

#endif
