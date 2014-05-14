#include <LTBL/Constructs/AABB.h>

#include <SFML/OpenGL.hpp>
#include <cstdio>
#include <algorithm>

AABB::AABB()
	: m_lowerBound(0.0f, 0.0f), m_upperBound(1.0f, 1.0f),
	m_center(0.5f, 0.5f), m_halfDims(0.5f, 0.5f)
{
}

AABB::AABB(const sf::Vector2f &lowerBound, const sf::Vector2f &upperBound)
	: m_lowerBound(lowerBound), m_upperBound(upperBound)
{
	calculateHalfDims();
	calculateCenter();
}

void AABB::calculateHalfDims()
{
	m_halfDims = (m_upperBound - m_lowerBound) / 2.0f;
}

void AABB::calculateCenter()
{
	m_center = m_lowerBound + m_halfDims;
}

void AABB::calculateBounds()
{
	m_lowerBound = m_center - m_halfDims;
	m_upperBound = m_center + m_halfDims;
}

AABB AABB::getRotatedAABB(float angleDegs) const
{
    //float angleRads = ltbl::deg2Rads(angleDegs);

	// get new dimensions
	//float cosOfAngle = cosf(angleRads);
	//float sinOfAngle = sinf(angleRads);

	AABB aabb = *this;

	aabb.setRotatedAABB(angleDegs);

    return aabb;
	//sf::Vector2f newHalfDims(m_halfDims.y * sinOfAngle + m_halfDims.x * cosOfAngle, m_halfDims.x * sinOfAngle + m_halfDims.y * cosOfAngle);

	//return AABB(m_center - newHalfDims, m_center + newHalfDims);
}


void AABB::setRotatedAABB(float angleDegs)
{
    float angleRads = ltbl::deg2Rads(angleDegs);
	float cosOfAngle = cos(angleRads);
	float sinOfAngle = sin(angleRads);

    //x_origin is the rotation point
    //x = ((x - x_origin) * cos(angle)) - ((y_origin - y) * sin(angle)) + x_origin
    //y = ((y_origin - y) * cos(angle)) - ((x - x_origin) * sin(angle)) + y_origin

    sf::Vector2f newLowerBound, newUpperBound;

    newLowerBound.x = ((m_lowerBound.x - m_center.x) * cosOfAngle) - ((m_center.y - m_lowerBound.y) * sinOfAngle) + m_center.x;
    newLowerBound.y = ((m_center.y - m_lowerBound.y) * cosOfAngle) - ((m_lowerBound.x - m_center.x) * sinOfAngle) + m_center.y;

    newUpperBound.x = ((m_upperBound.x - m_center.x) * cosOfAngle) - ((m_center.y - m_upperBound.y) * sinOfAngle) + m_center.x;
    newUpperBound.y = ((m_center.y - m_upperBound.y) * cosOfAngle) - ((m_upperBound.x - m_center.x) * sinOfAngle) + m_center.y;

    //if(newLowerBound.x < 0.f)
    //    newLowerBound.x *= -1.f;

    //if(newLowerBound.y < 0.f)
    //    newLowerBound.y *= -1.0f;

    //if(newUpperBound.x < 0.f)
    //    newUpperBound.x *= -1.f;

    if(newUpperBound.x < newLowerBound.x)
        std::swap<float>(newUpperBound.x, newLowerBound.x);

    if(newUpperBound.y < newLowerBound.y)
        std::swap<float>(newUpperBound.y, newLowerBound.y);

    m_lowerBound = newLowerBound;
    m_upperBound = newUpperBound;

    calculateHalfDims();
    //fprintf(stdout, "l:%f %f\nu:%f %f\n", m_lowerBound.x, m_lowerBound.y, m_upperBound.x, m_upperBound.y);

}

void AABB::setDims(const sf::Vector2f &newDims)
{
	setHalfDims(newDims / 2.0f);
}

sf::Vector2f AABB::getDims() const
{
	return m_upperBound - m_lowerBound;
}

void AABB::setHalfDims(const sf::Vector2f &newDims)
{
	m_halfDims = newDims;

	calculateBounds();
}

const sf::Vector2f &AABB::getHalfDims() const
{
	return m_halfDims;
}

void AABB::setBounds(const sf::Vector2f& newLowerBound, const sf::Vector2f& newUpperBound)
{
    m_lowerBound = newLowerBound;
    m_upperBound = newUpperBound;

    calculateHalfDims();
    calculateCenter();
}

void AABB::setLowerBound(const sf::Vector2f& newLowerBound)
{
    m_lowerBound = newLowerBound;

    calculateHalfDims();
	calculateCenter();
}

const sf::Vector2f &AABB::getLowerBound() const
{
	return m_lowerBound;
}

void AABB::setUpperBound(const sf::Vector2f& newUpperBound)
{
    m_upperBound = newUpperBound;

    calculateHalfDims();
	calculateCenter();
}

const sf::Vector2f &AABB::getUpperBound() const
{
	return m_upperBound;
}

void AABB::setCenter(const sf::Vector2f &newCenter)
{
	m_center = newCenter;

	calculateBounds();
}

void AABB::incCenter(const sf::Vector2f &increment)
{
	m_center += increment;

	calculateBounds();
}

const sf::Vector2f &AABB::getCenter() const
{
	return m_center;
}

bool AABB::intersects(const AABB &other) const
{
	if(m_upperBound.x < other.getLowerBound().x)
		return false;

	if(m_upperBound.y < other.getLowerBound().y)
		return false;

	if(m_lowerBound.x > other.getUpperBound().x)
		return false;

	if(m_lowerBound.y > other.getUpperBound().y)
		return false;

	return true;
}

bool AABB::contains(const AABB &other) const
{
	if(other.getLowerBound().x >= m_lowerBound.x &&
        other.getUpperBound().x <= m_upperBound.x &&
		other.getLowerBound().y >= m_lowerBound.y &&
		other.getUpperBound().y <= m_upperBound.y)
		return true;

	return false;
}

void AABB::debugRender() const
{
	// Render the AABB with lines
	//glTranslatef(0.0f, 600.f, 0.0);
	//glScalef(1.0f, -1.0f, 1.0);

	glBegin(GL_LINES);

	// Bottom
	glVertex2f(m_lowerBound.x, m_lowerBound.y);
	glVertex2f(m_upperBound.x, m_lowerBound.y);

	// Right
	glVertex2f(m_upperBound.x, m_lowerBound.y);
	glVertex2f(m_upperBound.x, m_upperBound.y);

	// Top
	glVertex2f(m_upperBound.x, m_upperBound.y);
	glVertex2f(m_lowerBound.x, m_upperBound.y);

	// Left
	glVertex2f(m_lowerBound.x, m_upperBound.y);
	glVertex2f(m_lowerBound.x, m_lowerBound.y);

	glEnd();
}
