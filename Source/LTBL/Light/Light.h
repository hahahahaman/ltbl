

#ifndef LTBL_LIGHT_H
#define LTBL_LIGHT_H

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <LTBL/Constructs.h>
#include <LTBL/QuadTree/QuadTree.h>

/**
*Base class of Light_Point...doesn't really do anything else...
*/

namespace ltbl
{
	class Light : public qdt::QuadTreeOccupant
	{
	private:
		sf::RenderWindow* m_pWin;

		sf::RenderTexture* m_pStaticTexture;

		// set up viewport information for the render texture
		void switchStaticTexture();

		bool m_alwaysUpdate;

		float m_intensity;
		float m_radius;
		float m_size;

		// If using light autenuation shader
		float m_bleed;
		float m_linearizeFactor;

        sf::Vector3f m_color;
	protected:
	    //fixing up an error that occurs from light_point spread angle
	    sf::Vector2f m_center;

	    bool m_updateRequired;

		class LightSystem* m_pLightSystem;

		// set to false in base classes in order to avoid shader attenuation
		bool m_shaderAttenuation;

	public:
		Light();
		virtual ~Light();

		//AABB* getAABB();
		virtual void calculateAABB();

		bool alwaysUpdate();
		void setAlwaysUpdate(bool always);

        //radius is how far light spreads
		void setRadius(float radius);
		void incRadius(float increment);
		float getRadius() const;

		virtual void setCenter(const sf::Vector2f& center);
		virtual void incCenter(const sf::Vector2f& increment);
		sf::Vector2f getCenter() const;

        //intensity does not do much, if it is <= 1 the light is not shown
		void setIntensity(float intensity);
		void incIntensity(float increment);
		float getIntensity() const;

        //Size affects how large shadow fins are and hull intersection
		void setSize(float sizea);
		void incSize(float increment);
		float getSize() const;

        //size of oblique light circle
		void setBleed(float bleed);
		void incBleed(float increment);
		float getBleed() const;

        //transparency of the light, 0.0f being transparent
		void setLinearizeFactor(float linearizeFactor);
		void incLinearizeFactor(float increment);
		float getLinearizeFactor() const;

		void setColor(const sf::Vector3f& color);
		sf::Vector3f getColor() const;

		virtual void renderLightSolidPortion() = 0;
		virtual void renderLightSoftPortion() = 0;

		friend class LightSystem;
	};
}

#endif
