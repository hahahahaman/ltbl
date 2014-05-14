#ifndef LTBL_CONVEX_HULL_H
#define LTBL_CONVEX_HULL_H

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/ConvexShape.hpp>

#include <LTBL/Constructs.h>
#include <LTBL/QuadTree/QuadTree.h>
#include <vector>

/**
*Convex Shaped object that blocks light
*
*sf::ConvexShape gives the object fillColor, outlineColor (neat stuff),
*
*call setRenderLightOverHull(false) to see the colors
*/

namespace ltbl
{
class ConvexHull : public sf::ConvexShape, public qdt::QuadTreeOccupant
{
    private:
        std::vector<sf::Vector2f> m_normals;

        bool m_render;

        sf::Vector2f m_worldCenter;

        bool m_aabbCalculated;

        bool m_updateRequired;

        float m_transparency;

        bool m_renderLightOverHull;

    public:
        //std::vector<sf::Vector2f> m_vertices;

        explicit ConvexHull(unsigned int pointCount = 0);
        virtual ~ConvexHull();

        void centerHull();

        bool loadShape(const char* fileName);

        sf::Vector2f getWorldVertex(unsigned int index) const;

        void calculateNormals();

        void renderHull(float depth);

        void calculateAABB();
        bool hasCalculatedAABB() const;

        void setWorldCenter(const sf::Vector2f &newCenter);
        void incWorldCenter(const sf::Vector2f &increment);
        sf::Vector2f getWorldCenter() const;

        void setTransparency(float transparency);
        void incTransparency(float increment);
        float getTransparency() const;

        //***Important
        //if true the hull is transparent
        //if false sf::ConvexShape colors work
        void setRenderLightOverHull(bool renderLightOverHull);
        bool getRenderLightOverHull() const;

        bool pointInsideHull(const sf::Vector2f &point);

        void debugDraw();

        friend class LightSystem;
};
}

#endif

