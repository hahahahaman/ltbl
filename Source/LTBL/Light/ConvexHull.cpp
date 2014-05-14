

#include <LTBL/Light/ConvexHull.h>
#include <LTBL/Utils.h>

#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>

namespace ltbl
{
ConvexHull::ConvexHull(unsigned int pointCount)
    : ConvexShape(pointCount),
      m_worldCenter(0.0f, 0.0f),
      m_aabbCalculated(false),
      m_updateRequired(true), // Remains true permanently unless user purposely changes it
      m_transparency(1.0f),
      m_renderLightOverHull(true)
{
}
ConvexHull::~ConvexHull()
{

}

void ConvexHull::centerHull()
{
    // calculate the average of all of the vertices, and then
    // offset all of them to make this the new origin position (0,0)
    const unsigned int numVertices = getPointCount();//m_vertices.size();

    sf::Vector2f posSum(0.0f, 0.0f);

    for(unsigned int i = 0; i < numVertices; i++)
        posSum += getPoint(i);

    sf::Vector2f averagePos(posSum / static_cast<float>(numVertices));

    for(unsigned int i = 0; i < numVertices; i++)
        setPoint(i, getPoint(i) - averagePos);

    calculateAABB();
}

bool ConvexHull::loadShape(const char* fileName)
{
    std::ifstream load(fileName);

    if(!load)
    {
        load.close();

        std::cout << "Could not load convex hull \"" << fileName << "\"!" << std::endl;

        return false;
    }
    else
    {
        while(!load.eof())
        {
            std::string firstElement, secondElement;

            load >> firstElement >> secondElement;

            if(firstElement.size() == 0 || secondElement.size() == 0)
                break;
            setPointCount(getPointCount() + 1);
            setPoint(getPointCount() - 1, sf::Vector2f(getFloatVal(firstElement), getFloatVal(secondElement)));
        }

        load.close();
    }

    centerHull();

    calculateNormals();

    calculateAABB();

    return true;
}

sf::Vector2f ConvexHull::getWorldVertex(unsigned int index) const
{
    assert(index >= 0 && index < getPointCount());
    //return sf::Vector2f(m_vertices[index].x + m_worldCenter.x, m_vertices[index].y + m_worldCenter.y);
    return getTransform().transformPoint(getPoint(index));
}

void ConvexHull::calculateNormals()
{
    const unsigned int numVertices = getPointCount();

    if(m_normals.size() != numVertices)
        m_normals.resize(numVertices);

    for(unsigned int i = 0; i < numVertices; i++) // dots are wrong
    {
        unsigned int index2 = i + 1;

        // wrap
        if(index2 >= numVertices)
            index2 = 0;

        m_normals[i].x = -(getPoint(index2).y - getPoint(i).y);
        m_normals[i].y = getPoint(index2).x - getPoint(i).x;
    }
}

void ConvexHull::renderHull(float depth)
{
    if(m_renderLightOverHull)
        return;

    glBegin(GL_TRIANGLE_FAN);

    const unsigned int numVertices = getPointCount();

    for(unsigned int i = 0; i < numVertices; i++)
    {
        sf::Vector2f vPos(getWorldVertex(i));
        glVertex3f(vPos.x, vPos.y, depth);
    }

    glEnd();
}

void ConvexHull::calculateAABB()
{
    assert(getPointCount() > 0);
    sf::FloatRect rect = getGlobalBounds();
    m_aabb.setLowerBound(sf::Vector2f(rect.left, rect.top));
    m_aabb.setUpperBound(sf::Vector2f(rect.left + rect.width, rect.top + rect.height));

    /*
    for(unsigned int i = 0; i < getPointCount(); i++)
    {
        sf::Vector2f* pPos = &getPoint(i);

        if(pPos->x > m_aabb.getUpperBound().x)
            m_aabb.setUpperBound(sf::Vector2f(pPos->x, m_aabb.getUpperBound().y));

        if(pPos->y > m_aabb.getUpperBound().y)
            m_aabb.setUpperBound(sf::Vector2f(m_aabb.getUpperBound().x, pPos->y));

        if(pPos->x < m_aabb.getLowerBound().x)
            m_aabb.setLowerBound(sf::Vector2f(pPos->x, m_aabb.getLowerBound().y));

        if(pPos->y < m_aabb.getLowerBound().y)
            m_aabb.setLowerBound(sf::Vector2f(m_aabb.getLowerBound().x, pPos->y));
    }
    */

    m_aabb.calculateHalfDims();
    m_aabb.calculateCenter();

    m_aabbCalculated = true;
}

bool ConvexHull::hasCalculatedAABB() const
{
    return m_aabbCalculated;
}

void ConvexHull::setWorldCenter(const sf::Vector2f &newCenter)
{
    m_worldCenter = newCenter;
    m_aabb.setCenter(m_worldCenter);

    setPosition(m_worldCenter);

    treeUpdate();
}

void ConvexHull::incWorldCenter(const sf::Vector2f &increment)
{
    m_worldCenter += increment;
    m_aabb.incCenter(increment);

    setPosition(m_worldCenter);

    treeUpdate();
}

sf::Vector2f ConvexHull::getWorldCenter() const
{
    return m_worldCenter;
}

void ConvexHull::setTransparency(float transparency)
{
    m_transparency = transparency;

    sf::Color color = getFillColor();
    setFillColor(sf::Color(color.r, color.g, color.b, m_transparency));
    color = getOutlineColor();
    setOutlineColor(sf::Color(color.r, color.g, color.b, m_transparency));
}

void ConvexHull::incTransparency(float increment)
{
    m_transparency += increment;

    sf::Color color = getFillColor();
    setFillColor(sf::Color(color.r, color.g, color.b, m_transparency));
    color = getOutlineColor();
    setOutlineColor(sf::Color(color.r, color.g, color.b, m_transparency));
}

float ConvexHull::getTransparency() const
{
    return m_transparency;
}

void ConvexHull::setRenderLightOverHull(bool renderLightOverHull)
{
    m_renderLightOverHull = renderLightOverHull;
}

bool ConvexHull::getRenderLightOverHull() const
{
    return m_renderLightOverHull;
}

bool ConvexHull::pointInsideHull(const sf::Vector2f &point)
{
    return getGlobalBounds().contains(point);
    /*
    int sgn = 0;

    for(unsigned int i = 0; i < getPointCount(); i++)
    {
        int wrappedIndex = wrap(i + 1, getPointCount());
        sf::Vector2f currentVertex(getWorldVertex(i));
        sf::Vector2f side(getWorldVertex(wrappedIndex) - currentVertex);
        sf::Vector2f toPoint(point - currentVertex);

        float cpd = cross2d(side, toPoint);

        int cpdi = static_cast<int>(cpd / std::abs(cpd));

        if(sgn == 0)
            sgn = cpdi;
        else if(cpdi != sgn)
            return false;
    }
    return true;
     */
}

void ConvexHull::debugDraw()
{
    const unsigned int numVertices = getPointCount();

    glTranslatef(m_worldCenter.x, m_worldCenter.y, 0.0f);

    glBegin(GL_LINE_LOOP);

    for(unsigned int i = 0; i < numVertices; i++)
        glVertex2f(getPoint(i).x, getPoint(i).y);

    glEnd();
}
}
